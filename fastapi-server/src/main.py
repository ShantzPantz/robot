import asyncio
import io
import time
from typing import Union
import wave
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from starlette.websockets import WebSocketState
# from lib.AudioProcessor import AudioProcessor
from lib.SpeechToTextProcessor import SpeechToTextProcessor
from vosk import KaldiRecognizer, Model
import edge_tts
import av

app = FastAPI()

# Create a global instance of the audio processor
# Initialize it with your model paths.
# processor = AudioProcessor(
#     ow_model_path="path/to/your/openwakeword_model.tflite",
#     vosk_model_path="data/vosk-models/vosk-model-en-us-0.22"
# )

vosk_model = Model("data/vosk-models/vosk-model-en-us-0.22")
rec = KaldiRecognizer(vosk_model, 16000)

# edge tts
VOICE = "en-GB-ThomasNeural"

TEST_SENTENCES = [
    "Hello there! This is the first test sentence.",
    "How are you doing today?",
    "The quick brown fox jumps over the lazy dog.",
    "Numbers test: one, two, three, four, five, six, seven, eight, nine, ten.",
    "Punctuation check: Wait—what?! Really... no way!",
    "This is a slightly longer sentence designed to test how the text to speech system handles more continuous speech without pauses.",
    "Time test: It is now 10:30 in the morning.",
    "Address test: 123 Main Street, Apartment 4B, Springfield.",
    "Date test: Today is Friday, August twenty-second, two thousand and twenty-five.",
    "Speed test: She sells seashells by the seashore.",
    "Math test: Two plus two equals four, and five times five equals twenty-five.",
    "Emotion test: I am so excited! But wait, I am also a little nervous.",
    "Foreign words: Bonjour, hola, konnichiwa, guten tag.",
    "Spelling test: C-A-T spells cat, D-O-G spells dog.",
    "Goodbye for now. This is the last test sentence in the cycle."
]

@app.websocket("/audio_test")
async def audio_test(websocket: WebSocket):    
    await websocket.accept()

    last_msg = 0
    last_ping = 0
    index = 0
    
    speech = SpeechToTextProcessor(rec)
    
    # output_buffer = b""
    try:
        while True:
            await asyncio.sleep(0) 

            message = await websocket.receive()        
             # Check the message type
            if message["type"] == "websocket.receive":
                # Handle incoming text or binary data
                if "text" in message:
                    print(f"Received text message: {message['text']}")
                # /elif "bytes" in message:
                    # print(f"Received binary message of {len(message['bytes'])} bytes.")
            elif message["type"] == "websocket.ping":
                # This block will be executed when a ping frame is received
                print("Received a ping message.")
            elif message["type"] == "websocket.pong":
                # This block will be executed when a pong frame is received
                print("Received a pong message.")                 

            now = time.time()           

            if message.get("bytes") is not None:                
                frame = message["bytes"]                
                text = speech.process_incoming_bytes(frame)            
                if text:                                    
                    print(f"Sending: {text}", flush=True)
                    communicate = edge_tts.Communicate(text, VOICE, rate="+0%", pitch="+0Hz")               
                    decoder = av.CodecContext.create("mp3", "r")

                    # Set up a Resampler to convert audio to the desired format
                    resampler = av.AudioResampler(
                        format="s16",  # 16-bit signed integers
                        layout="mono", # Single channel
                        rate=16000     # 16kHz sample rate
                    )
                
                    # stream is returned in mp3 format, but esp32 expects wav.
                    async for chunk in communicate.stream():
                        if chunk["type"] != "audio":
                            continue                    
                        
                        mp3_data = chunk["data"]

                        packets = decoder.parse(mp3_data)
                        for packet in packets:
                            frames = decoder.decode(packet)
                            for frame in frames:
                                # resample can return a single frame, a list of frames, or None
                                resampled_frames = resampler.resample(frame)

                                # Handle cases where resample returns a list
                                if resampled_frames:
                                    # Ensure we iterate, even if it's a list with one item
                                    if not isinstance(resampled_frames, list):
                                        resampled_frames = [resampled_frames]

                                    for resampled_frame in resampled_frames:
                                        pcm_bytes = resampled_frame.to_ndarray().tobytes()
                                        await websocket.send_bytes(pcm_bytes)               


                    
                

            # message = await websocket.receive()
            # if message.get("bytes") is not None:                
            #     frame = message["bytes"]                
            #     output_buffer += frame
            #     text = speech.process_incoming_bytes(frame)            
            #     if text:
            #         await websocket.send_text(text)
            #         await websocket.send_bytes(output_buffer)
            #         output_buffer = b""
                    # await websocket.send_bytes(frame)

    except WebSocketDisconnect:
        print("Client disconnected cleanly", flush=True)
    except Exception as e:
        print(f"Error: {e}", flush=True)
    finally:
        if websocket.client_state != WebSocketState.DISCONNECTED:
            await websocket.close()
        print("WebSocket connection closed.", flush=True)


# @app.websocket("/audio_test_old")
# async def audio_test_old(websocket: WebSocket):
#     AUDIO_SIZE = 16000 * 10 * 2 # 16,000 samples x 2 bytes x 10 seconds
#     buff = b""

#     await websocket.accept()
#     print("WebSocket Audio Test.", flush=True)
#     try:
#         while True:
#             message = await websocket.receive()
            
#             # Process only binary frames
#             if message.get("bytes") is not None:
#                 data = message["bytes"]
#                 processor.test_bytes(data)
#                 await websocket.send_bytes(data)
#                 # buff += data
#                 # if len(buff) >= AUDIO_SIZE:
#                 #     # await websocket.send_text("Buffer is full.")
#                 #     await websocket.send_bytes(buff)
#                 #     buff = b""                

#             # Process text separately
#             elif message.get("type") == "websocket.receive" and "text" in message:
#                 text_data = message["text"]
#                 print(f"Received text data: {text_data}")
#                 await websocket.send_text("got a message.")          

#     except WebSocketDisconnect:
#         print("Client disconnected cleanly", flush=True)
#     except Exception as e:
#         print(f"Error: {e}", flush=True)
#     finally:
#         if websocket.client_state != WebSocketState.DISCONNECTED:
#             await websocket.close()
#         print("WebSocket connection closed.", flush=True)

# @app.websocket("/ws")
# async def websocket_endpoint(websocket: WebSocket):
#     await websocket.accept()
#     print("WebSocket connection accepted.")
#     try:
#         while True:
#             message = await websocket.receive()
            
#             # Process only binary frames
#             if message.get("type") == "websocket.receive" and "bytes" in message:
#                 data = message["bytes"]
#                 # call handle_bytes asynchronously if possible
#                 # e.g., offload heavy work
#                 # processor.handle_bytes(data)                
#                 # output = processor.handle_bytes(data)
#                 # await websocket.send_text("WTF")
#                 # if output:
#                 #     await websocket.send_text(output)
#                 output = processor.handle_bytes(data)
#                 processor.write_to_file(data)

#                 # TODO if processor.has_text(), or is_ready(), some indicator that text is ready to be processed
                
                
#                 # await websocket.send_text("test")

#             # Process text separately
#             elif message.get("type") == "websocket.receive" and "text" in message:
#                 text_data = message["text"]
#                 print(f"Received text data: {text_data}")
#                 await websocket.send_text("got a message.")

#             # Give control back to asyncio loop to process TCP
#             await asyncio.sleep(0)  

#     except Exception as e:
#         print(f"Error: {e}")
#     finally:
#         if websocket.client_state != WebSocketState.DISCONNECTED:
#             await websocket.close()
#             processor.close_file()
#         print("WebSocket connection closed.")
