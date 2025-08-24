from collections import deque
import json
import time
import wave
import numpy as np
from openwakeword.model import Model as OWModel
from vosk import Model as VoskModel, KaldiRecognizer
import openwakeword
import traceback
import noisereduce as nr


# Ensure models are downloaded
openwakeword.utils.download_models()

class AudioProcessor:
    OW_FRAME_SIZE = 2560  # 80ms @16kHz, 16-bit mono
    VOSK_FRAME_SIZE = OW_FRAME_SIZE * 15

    # Parameters
    SAMPLE_WIDTH = 2        # bytes per sample (16-bit PCM)
    WINDOW_SIZE = 512       # number of samples in each analysis window
    HOP_SIZE = 256          # how far to slide each time (overlap = WINDOW_SIZE - HOP_SIZE)

    def __init__(
        self,
        ow_model_path="",
        vosk_model_path="data/vosk-models/vosk-model-small-en-us-0.15",
        wake_word_name="alexa",
        output_folder="/app/audio"
    ):
        print("AudioProcessor initialized.")
        
        self.wake_word_name = wake_word_name
        self.transcription_active = False        

        # Initialize OpenWakeWord
        self.ow_model = OWModel()
        self.ow_buff = b""

        # # Initialize Vosk
        self.vosk_buff = b""
        self.vosk_model = VoskModel( vosk_model_path )
        self.vosk_recognizer = KaldiRecognizer(self.vosk_model, 16000)

    

    def test_bytes(self, frame: bytes):            
        # pass to wake word engine        
        if not self.transcription_active:        
            audio_array = np.frombuffer(frame, dtype=np.int16)
            prediction = self.ow_model.predict(audio_array)
            score = prediction.get(self.wake_word_name, 0.0)
            if score > 0.5:
                print(f"Wake word '{self.wake_word_name}' detected! Activating transcription...", flush=True)            
                self.transcription_active = True
        
        if self.transcription_active:
            self.vosk_buff += frame
            if len(self.vosk_buff) >= self.VOSK_FRAME_SIZE:
                output = self.process_vosk_frame(self.vosk_buff)
                if output:
                    print(output, flush=True)
                self.vosk_buff = self.vosk_buff[self.VOSK_FRAME_SIZE:]
                
            
    def process_vosk_frame(self, frame: bytes):        
        output = ""
        try:
            if self.vosk_recognizer.AcceptWaveform(frame):
                result = json.loads(self.vosk_recognizer.Result())
                text = result.get("text", "")
                if text:                   
                    # Reset transcription state for next command
                    self.transcription_active = False
                    self.vosk_recognizer.Reset()
                    output = text                    
            else:
                partial_result = json.loads(self.vosk_recognizer.PartialResult())
                partial_text = partial_result.get("partial", "")
                if partial_text:
                    print(f"Partial: {partial_text}", flush=True)                             

        except Exception as e:
            print(f"⚠️ Exception in Vosk processing: {e}", flush=True)
            traceback.print_exc()
        return output
   

    def handle_bytes(self, frame: bytes):
        """Process exactly one frame (2560 bytes)."""
        output = ""

        if len(frame) != self.OW_FRAME_SIZE:
            print(f"⚠️ Dropped packet of unexpected size: {len(frame)}", flush=True)
            return
                
        try:
            audio_array = np.frombuffer(frame, dtype=np.int16)            

            # Wake-word detection
            if not self.transcription_active:
                prediction = self.ow_model.predict(audio_array)
                score = prediction.get(self.wake_word_name, 0.0)
                if score > 0.5:
                    print(f"Wake word '{self.wake_word_name}' detected! Activating transcription...", flush=True)                                        
                    # Include the wake word frame in the vosk buffer for context
                    self.vosk_buff += frame
                    self.transcription_active = True

            # Streaming transcription with Vosk
            if self.transcription_active:
                self.vosk_buff += frame 

                if len(self.vosk_buff) >= self.VOSK_FRAME_SIZE:
                    output = self.process_vosk_frame(self.vosk_buff[:self.VOSK_FRAME_SIZE])
                    self.vosk_buff = self.vosk_buff[self.VOSK_FRAME_SIZE:]

                    if output:
                        print("Output: " + output, flush=True)
                        self.transcription_active = False
        except Exception as e:            
            print(f"⚠️ Exception in handle_bytes processing: {e}", flush=True)
            traceback.print_exc()
               
        return output

