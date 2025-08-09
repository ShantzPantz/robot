from flask import Flask, request, send_file, Response, jsonify, abort
from PIL import Image 
import asyncio
import edge_tts
import io
import uuid
import os
from flask_cors import CORS
from openai_helpers import analyze_image, test_response
import base64
from vosk import Model, KaldiRecognizer
import wave 
import json
import time 

UPLOAD_FOLDER = "uploads"
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

vosk_model_path = "data/vosk-models/vosk-model-small-en-us-0.15"
vosk_model = Model(vosk_model_path)

app = Flask(__name__)
CORS(app) 
# Text-to-speech using edge-tts and save to in-memory file
async def synthesize_speech(text: str) -> io.BytesIO:
    # file_name = f"/tmp/{uuid.uuid4()}.mp3"
    # tts = edge_tts.Communicate(text, voice="en-US-AriaNeural")  # Customize voice as needed
    tts = edge_tts.Communicate(text, voice="en-GB-RyanNeural")  
    audio = bytearray()
    async for chunk in tts.stream():
        if chunk["type"] == "audio":
            audio.extend(chunk["data"])
    return bytes(audio)

def transcribe_wavefile(wf):   
    if wf.getnchannels() != 1 or wf.getsampwidth() != 2:
        return jsonify({"error": "Audio must be WAV mono PCM 16-bit."}), 400

    rec = KaldiRecognizer(vosk_model, wf.getframerate())
    transcription = ""

    while True:
        data = wf.readframes(4000)
        if not data:
            break
        if rec.AcceptWaveform(data):
            result = json.loads(rec.Result())
            transcription += result.get("text", "") + " "

    transcription += json.loads(rec.FinalResult()).get("text", "")
    return transcription.strip()

@app.route("/", methods=["GET", "POST"])
def hello_world():
    return "<p>Hello, World!</p>"

@app.route("/image_upload", methods=["POST", "GET"])
def image_upload():    
    if not request.data:
        abort(400, description="No image data received.")

    # Store all files for this request together.
    rid = request.args.get("rid")
    filename = str(int(time.time())) + ".jpg"
    request_folder = os.path.join(UPLOAD_FOLDER, rid)
    os.makedirs(request_folder, exist_ok=True)
    save_path = os.path.join(request_folder, filename)

    with open(save_path, "wb") as f:
        f.write(request.data)

    print(f"Saved raw image to {save_path}")
    return jsonify({"status": "success", "filename": filename})

@app.route("/communicate", methods=["POST"])
def communicate():
    txt_command = request.form.get("message", "")
    image_file = request.files.get("image")
    audio_file = request.files.get("audio")

    if not image_file:
        abort(404)
        return "No image provided."

    if not audio_file and not txt_command:
        abort(404)
        return "No command found."
    
    if audio_file:
        audio_path = os.path.join(UPLOAD_FOLDER, audio_file.filename)
        audio_file.save(audio_path)
        wf = wave.open(audio_path, "rb")
        command = transcribe_wavefile(wf)
    else:
        command = txt_command
    
    if image_file:
        # image = Image.open(image_file.stream)
        image_bytes = image_file.read()
        base64_image = base64.b64encode(image_bytes).decode('utf-8')

        print("Analyzing image")
        response = ""
        try: 
            response = analyze_image("gary", base64_image, command)
            # Generate TTS audio with edge-tts
            audio_bytes = asyncio.run(synthesize_speech(response))
            print("Response")
            print(response)
        except Exception as e: 
            audio_bytes = asyncio.run(synthesize_speech("There was an error with our communication."))
            print("Error with analyze image")
            print(e)
        
        return Response(audio_bytes, mimetype="audio/mpeg")
    

