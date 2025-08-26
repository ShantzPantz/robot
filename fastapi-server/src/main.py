import asyncio
import numpy as np
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from starlette.websockets import WebSocketState
from vosk import KaldiRecognizer, Model
from openwakeword.model import Model as OWModel
import av
import edge_tts

from lib.SpeechToTextProcessor import SpeechToTextProcessor
from lib.openai_helpers import generate_response

app = FastAPI()

# --- Models / Config
vosk_model = Model("data/vosk-models/vosk-model-en-us-0.22")
rec = KaldiRecognizer(vosk_model, 16000)

ow_model = OWModel(wakeword_models=["data/ow-models/hey_Marvin.tflite"])
ow_wake_word = "hey_Marvin"
OW_FRAME_SIZE = 2560

VOICE = "en-GB-ThomasNeural"
OPENAI_PERSONALITY = "marvin"

STATE_WAITING = "waiting"
STATE_TRANSCRIBING = "transcribing"
STATE_RESPONDING = "responding"


async def handle_response(websocket: WebSocket, text: str, state: dict):
    """Generate response, TTS, and stream back audio"""
    try:
        response = generate_response(OPENAI_PERSONALITY, text)

        communicate = edge_tts.Communicate(response, VOICE)
        decoder = av.CodecContext.create("mp3", "r")
        resampler = av.AudioResampler(format="s16", layout="mono", rate=16000)

        async for chunk in communicate.stream():
            if chunk["type"] != "audio":
                continue

            mp3_data = chunk["data"]
            packets = decoder.parse(mp3_data)
            for packet in packets:
                frames = decoder.decode(packet)
                for frame in frames:
                    resampled_frames = resampler.resample(frame)
                    if resampled_frames:
                        if not isinstance(resampled_frames, list):
                            resampled_frames = [resampled_frames]
                        for resampled_frame in resampled_frames:
                            pcm_bytes = resampled_frame.to_ndarray().tobytes()
                            await websocket.send_bytes(pcm_bytes)

    except Exception as e:
        print(f"Error in handle_response: {e}", flush=True)
    finally:
        # Reset state so loop starts listening again
        state["value"] = STATE_WAITING
        print("Done responding. Back to WAITING.", flush=True)


@app.websocket("/audio_test")
async def audio_test(websocket: WebSocket):
    await websocket.accept()
    print("Client connected.", flush=True)

    state = {"value": STATE_WAITING}
    speech = SpeechToTextProcessor(rec)
    ow_buff = b""

    try:
        while True:
            message = await websocket.receive()

            if message["type"] == "websocket.ping":
                print("Ping")
                continue
            elif message["type"] == "websocket.pong":
                print("Pong")
                continue

            frame = message.get("bytes")
            if frame is None:
                continue

            # ---- STATE: WAITING (wake word detection)
            if state["value"] == STATE_WAITING:
                ow_buff += frame                
                if len(ow_buff) >= OW_FRAME_SIZE:                    
                    audio_array = np.frombuffer(ow_buff[:OW_FRAME_SIZE], dtype=np.int16)                    
                    prediction = ow_model.predict(audio_array)                    
                    ow_buff = ow_buff[OW_FRAME_SIZE:]                   
                    score = prediction.get(ow_wake_word, 0.0)                    
                    if score > 0.5:
                        print(f"Wake word '{ow_wake_word}' detected! Score: {score}", flush=True)                        
                        state["value"] = STATE_TRANSCRIBING
                        ow_buff = b""

            # ---- STATE: TRANSCRIBING (speech to text)
            elif state["value"] == STATE_TRANSCRIBING:
                text = speech.process_incoming_bytes(frame)
                if text and text != "the":
                    print(f"Transcribed: {text}", flush=True)
                    state["value"] = STATE_RESPONDING

                    # spawn background task (non-blocking)
                    asyncio.create_task(handle_response(websocket, text, state))

            # ---- STATE: RESPONDING
            elif state["value"] == STATE_RESPONDING:
                # Drop or buffer audio frames here.
                # For now: just ignore incoming audio while responding.
                continue

            await asyncio.sleep(0)  # give control back to event loop

    except WebSocketDisconnect:
        print("Client disconnected cleanly", flush=True)
    except Exception as e:
        print(f"Error in websocket: {e}", flush=True)
    finally:
        if websocket.client_state != WebSocketState.DISCONNECTED:
            await websocket.close()
        print("WebSocket connection closed.", flush=True)
