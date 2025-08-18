import asyncio
import time
from typing import Union
import wave
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from starlette.websockets import WebSocketState
from lib.AudioProcessor import AudioProcessor

app = FastAPI()

# Create a global instance of the audio processor
# Initialize it with your model paths.
processor = AudioProcessor(
    ow_model_path="path/to/your/openwakeword_model.tflite",
    vosk_model_path="data/vosk-models/vosk-model-en-us-0.22"
)

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    print("WebSocket connection accepted.")
    try:
        while True:
            message = await websocket.receive()
            
            # Process only binary frames
            if message.get("type") == "websocket.receive" and "bytes" in message:
                data = message["bytes"]
                # call handle_bytes asynchronously if possible
                # e.g., offload heavy work
                # processor.handle_bytes(data)                
                # output = processor.handle_bytes(data)
                # await websocket.send_text("WTF")
                # if output:
                #     await websocket.send_text(output)
                output = processor.handle_bytes(data)
                
                # await websocket.send_text("test")

            # Process text separately
            elif message.get("type") == "websocket.receive" and "text" in message:
                text_data = message["text"]
                print(f"Received text data: {text_data}")
                await websocket.send_text("got a message.")

            # Give control back to asyncio loop to process TCP
            await asyncio.sleep(0)  

    except Exception as e:
        print(f"Error: {e}")
    finally:
        if websocket.client_state != WebSocketState.DISCONNECTED:
            await websocket.close()
        print("WebSocket connection closed.")
