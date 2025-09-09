import asyncio
import base64
import time
import numpy as np
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from starlette.websockets import WebSocketState
from vosk import KaldiRecognizer, Model
from openwakeword.model import Model as OWModel
import av
import edge_tts

from lib.SpeechToTextProcessor import SpeechToTextProcessor
from lib.openai_helpers import generate_response, respond_to_image

app = FastAPI()

# --- Models / Config
vosk_model = Model("data/vosk-models/vosk-model-en-us-0.22")
rec = KaldiRecognizer(vosk_model, 16000)

ow_model = OWModel(wakeword_models=["data/ow-models/hey_billy.tflite"])
ow_wake_word = "hey_billy"
# OW_FRAME_SIZE = 2560
OW_FRAME_SIZE = 1280
WAKEWORD_COOLDOWN_SECS = 3.0


# VOICE = "en-GB-ThomasNeural"
VOICE = "en-US-GuyNeural"   
OPENAI_PERSONALITY = "billy"

STATE_WAITING = "waiting"
STATE_WAITING_FOR_PLAYBACK = "waiting_for_playback"
STATE_TRANSCRIBING = "transcribing"
STATE_RESPONDING = "responding"
THRESHOLD = 0.75


# global connection mananger
# robot_id -> dict[client_id]
active_connections = {}
images = {}

# if this robot has a vision connect, ask it for an image
async def request_image(robot_id: str): 
    try:
        if robot_id not in active_connections:
            print("Robot ID not found.")
            return 
        if 'vision' not in active_connections[robot_id]:
            print("vision client not found.")
            return 
        webSocket = active_connections[robot_id]["vision"]
        await webSocket.send_text("request_image")
    except Exception as e:
        print(f"Error in request_image: {e}", flush=True)
    

async def handle_response(websocket: WebSocket, text: str, state: dict, base64_images: list = None, ow_buff: bytearray = None):
    """Generate response, TTS, and stream back audio"""
    try:
        response = ""
        if base64_images == None or len(base64_images) == 0:
            response = generate_response(OPENAI_PERSONALITY, text)
        elif len(base64_images) > 0:
            last_image = base64_images[-1]
            response = respond_to_image(OPENAI_PERSONALITY, last_image, text)

        print(response, flush=True)
        
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
        await websocket.send_text("cmd:end_of_stream")
        state["value"] = STATE_WAITING_FOR_PLAYBACK
        state["last_tts_end"] = time.time()
        print("Done responding. Back to WAITING.", flush=True)
        
        if ow_buff is not None:
            ow_buff.clear()


@app.websocket("/robot_vision")
async def robot_vision(websocket: WebSocket):
    await websocket.accept()
   

    robot_id = websocket.query_params.get("robot_id")
    if not robot_id:
        print("Connection rejected: No robot ID provided.", flush=True)
        await websocket.close(code=1008)
        return    

    print(f"Client {robot_id} -> vision connected.", flush=True)
    
    if not robot_id in active_connections: 
        active_connections[robot_id] = {}
    if not "vision" in active_connections[robot_id]:
        active_connections[robot_id]["vision"] = websocket

    try:
        while True: 
            await asyncio.sleep(0)
            message = await websocket.receive()

            if message["type"] == "websocket.ping":
                print("Ping")
                continue
            elif message["type"] == "websocket.pong":
                print("Pong")
                continue
            
            img_data = message.get("bytes")
            if img_data is None:
                continue
            
            if robot_id not in images:
                images[robot_id] = {"images": []}
            
            base64_image = base64.b64encode(img_data).decode("utf-8")
            images[robot_id]["images"].append(base64_image)

            print("got image data!", flush=True)
            filename = f"/app/uploads/ws_image_{int(time.time())}.jpg"
            try:
                with open(filename, "wb") as f:
                    f.write(img_data)
                print(f"Image successfully saved to {filename}", flush=True)
            except IOError as e:
                print(f"Error saving image: {e}")
                        # convert image to data to base64 that can be used with chatgpt. Store in images[robot_id]
    except WebSocketDisconnect:
        print("Client disconnected cleanly", flush=True)
    except Exception as e:
        print(f"Error in websocket: {e}", flush=True)
    finally:
        if websocket.client_state != WebSocketState.DISCONNECTED:
            await websocket.close()
        print("WebSocket connection closed.", flush=True)


@app.websocket("/robot_audio")
async def robot_audio(websocket: WebSocket):
    robot_id = websocket.query_params.get("robot_id")
    if not robot_id:
        print("Connection rejected: No robot ID provided.", flush=True)
        await websocket.close(code=1008)
        return    
    
    if not robot_id in active_connections: 
        active_connections[robot_id] = {}
    if not "audio" in active_connections[robot_id]:
        active_connections[robot_id]["audio"] = websocket
    
    await websocket.accept()
    print(f"Client {robot_id} -> audio connected.", flush=True)

    state = {"value": STATE_WAITING}
    speech = SpeechToTextProcessor(rec)
    ow_buff = bytearray()

    try:
        while True:
            message = await websocket.receive()

            if message["type"] == "websocket.ping":
                print("Ping")
                continue
            elif message["type"] == "websocket.pong":
                print("Pong")
                continue
            elif message["type"] == "websocket.receive" and "text" in message:
                if message["text"] == "cmd:end_of_playback":
                    print("Got end of playback msg. WAITING", flush=True)                                                            
                    ow_buff.clear()
                    state["value"] = STATE_WAITING                    
                
            frame = message.get("bytes")
            if frame is None:
                continue

            # ---- STATE: WAITING (wake word detection)
            if state["value"] == STATE_WAITING:
                if "last_tts_end" in state and time.time() - state["last_tts_end"] < WAKEWORD_COOLDOWN_SECS:
                    continue  # skip wakeword detection during cooldown

                ow_buff.extend(frame)          
                if len(ow_buff) >= OW_FRAME_SIZE:                    
                    audio_array = np.frombuffer(ow_buff[:OW_FRAME_SIZE], dtype=np.int16)                    
                    prediction = ow_model.predict(audio_array)                    
                    ow_buff = ow_buff[OW_FRAME_SIZE:]                   
                    score = prediction.get(ow_wake_word, 0.0)                    
                    if score > THRESHOLD:
                        print(f"Wake word '{ow_wake_word}' detected! Score: {score}", flush=True)                          
                        
                        # # ask for an image for vision client
                        asyncio.create_task(request_image(robot_id))
                        
                        state["value"] = STATE_TRANSCRIBING

                        start_time = time.time()
                        FLUSH_SIZE = 32000
                        while time.time() - start_time < 2:
                            prediction = ow_model.predict(np.zeros(FLUSH_SIZE))
                            positive_detections = {key: value for key, value in prediction.items() if value > THRESHOLD}
                            if (len(positive_detections) == 0):
                                break
                            await asyncio.sleep(0.1)

                        
            # ---- STATE: TRANSCRIBING (speech to text)
            elif state["value"] == STATE_TRANSCRIBING:
                text = speech.process_incoming_bytes(frame)
                if text and text != "the":
                    print(f"Transcribed: {text}", flush=True)
                    state["value"] = STATE_RESPONDING
                    ow_buff.clear()

                    # spawn background task (non-blocking)
                    # get images for this robot id 
                    robot_images = images.get(robot_id, {}).get("images", None)     
                    if robot_images != None:               
                        asyncio.create_task(handle_response(websocket, text, state, base64_images=robot_images.copy()))                    
                        # clear image cache
                        images[robot_id]["images"] = []
                    

            # ---- STATE: RESPONDING
            elif state["value"] == STATE_RESPONDING or state["value"] == STATE_WAITING_FOR_PLAYBACK:
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
