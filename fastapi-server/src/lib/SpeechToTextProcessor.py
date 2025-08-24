import json 
import webrtcvad

class SpeechToTextProcessor:
    def __init__(self, vosk_recognizer, sample_rate=16000, frame_ms=30, 
                      vad_aggressiveness=2, silence_ms=600):
        self.rec = vosk_recognizer
        self.vad = webrtcvad.Vad(vad_aggressiveness)

        self.sample_rate = sample_rate
        self.frame_ms = frame_ms
        self.frame_bytes = int(sample_rate * frame_ms / 1000) * 2 # int16 = 2 bytes

        # consecutive non-speach frames for "end of speech"
        self.silence_frames_needed = max(1, silence_ms // frame_ms)

        self.buffer = bytearray()
        self.nonspeech_run = 0
        self.saw_speech_this_utt = False

    def process_30ms_frame(self, frame_30ms: bytes):
        output = ""

        # check if speech
        is_speech = self.vad.is_speech(frame_30ms, self.sample_rate)

        # print("Speaking" if is_speech else "Silence", flush=True)

        # feed frame to vosk, if we have output, send it back
        if self.rec.AcceptWaveform(frame_30ms):
            result = json.loads(self.rec.Result())
            text = result.get("text", "")
            if text:
                output = text 
                self.rec.Reset()
                self.nonspeech_run = 0
                self.saw_speech_this_utt = False 
            return output
        
        # not a full phrase, partial results
        partial = json.loads(self.rec.PartialResult() or "{}")
        if partial.get("partial"):
            # print("Partial: ", partial["partial"], flush=True)
            pass

        # check for speech
        if is_speech:
            self.saw_speech_this_utt = True
            self.nonspeech_run = 0
        else: 
            self.nonspeech_run += 1
            
            # if we have the threshold of silence after speech started, flush it
            if self.saw_speech_this_utt and self.nonspeech_run >= self.silence_frames_needed:
                final = json.loads(self.rec.FinalResult())
                text = final.get("text", "")
                if text:
                    output = text 
                #  reset 
                self.rec.Reset()
                self.nonspeech_run = 0
                self.saw_speech_this_utt = False 

        return output


    def process_incoming_bytes(self, frame: bytes):
        outputs = []
        self.buffer.extend(frame)

        # slice into 30ms chunks for VAD
        while len(self.buffer) >= self.frame_bytes:
            frame_30ms = self.buffer[:self.frame_bytes]
            del self.buffer[:self.frame_bytes]
            text = self.process_30ms_frame(bytes(frame_30ms))
            if text:
                outputs.append(text)
        
        # return any commited phrases. 
        return " ".join(outputs)

    