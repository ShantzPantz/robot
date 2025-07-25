from flask import Flask, request, send_file
from PIL import Image 
import asyncio
import edge_tts
import io
import uuid
import os
from flask_cors import CORS
from openai_helpers import analyze_image, test_response
import base64

app = Flask(__name__)
CORS(app) 
# Text-to-speech using edge-tts and save to in-memory file
async def synthesize_speech(text: str) -> io.BytesIO:
    file_name = f"/tmp/{uuid.uuid4()}.mp3"
    tts = edge_tts.Communicate(text, voice="en-US-AriaNeural")  # Customize voice as needed
    await tts.save(file_name)

    # Read back into BytesIO and delete file
    audio_bytes = io.BytesIO()
    with open(file_name, "rb") as f:
        audio_bytes.write(f.read())
    os.remove(file_name)
    audio_bytes.seek(0)
    return audio_bytes

@app.route("/")
def hello_world():
    return "<p>Hello, World!</p>"

@app.route("/upload", methods=["POST"])
def upload():
    
    if 'image' not in request.files:
        return "No image provided", 400

    # Load and process image (example stub)
    image_file = request.files['image']
    # image = Image.open(image_file.stream)
    image_bytes = image_file.read()
    base64_image = base64.b64encode(image_bytes).decode('utf-8')
    
    #TODO send image to OpenAI image processing with a custom command. 
    if 'message' in request.form:
        command = request.form['message']
    else:        
        command = "Tell me what is going on in this image."

    print("Analyzing image")
    response = "testing"
    try: 
        response = analyze_image("dougie", base64_image, command)
        print("Response")
        print(response)
    except Exception as e: 
        print("Error with analyze image")
        print(e)
    
    # Generate TTS audio with edge-tts
    audio_bytes = asyncio.run(synthesize_speech(response))

    return send_file(
        audio_bytes,
        mimetype="audio/mpeg",
        download_name="response.mp3"
    )
    