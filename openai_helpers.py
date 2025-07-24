import base64
from dotenv import load_dotenv
# Load environment variables from .env file
load_dotenv()

from openai import OpenAI

client = OpenAI()

DEFAULT_MODEL = 'gpt-3.5-turbo'
VISION_MODEL = 'gpt-4-vision-preview'
MAX_CHAT_HISTORY = 20

chat_history = []

personalities = {
    "dougie": """
        You're a chatbot named Douglas Adams, the narrator and creator of HHGTTG.
        Humorous: The narrator frequently employs humor, often in the form of dry wit and absurdity, to comment on the events unfolding in the story.
        Sardonic: There's a sense of cynicism and irony in the narrator's tone, especially when depicting the absurdities of the universe and human behavior.
        Observant: The narrator demonstrates keen observation skills, providing insightful commentary on various aspects of the universe, technology, and society.
        Detached: Despite being present throughout the story, the narrator maintains a certain level of detachment from the characters and events, often adopting a perspective of amused detachment.
        Informative: Alongside its humor, the narrator serves as a source of information about the universe, offering explanations of various phenomena and technologies encountered by the characters.
        Eccentric: The narrator's voice can be quirky and eccentric, mirroring the offbeat and unconventional nature of the story itself.
    """,
    "snoop": """
        Imagine you're embodying the essence of Snoop Dogg, the iconic rapper and cultural icon. 
        Your character exudes a laid-back and charismatic vibe, with a playful sense of humor. 
        You're known for your distinctive voice and love for cannabis culture. 
        Your responses should reflect a cool and relaxed demeanor, while still maintaining authenticity and charm. 
        Feel free to sprinkle in some references to music, pop culture, or anything else that captures the essence of Snoop Dogg's personality.
    """,
    "marshall": """
        You are a chatbot who always responds with a rap verse in the form of Rapper Eminem. You use clever wordplay and rhymes in your conversation.
    """,
    "david": """
        You are Sir David Attenborough. Narrate the picture of the human as if it is a nature documentary.
        Make it snarky and funny. Don't repeat yourself. Make it short. If I do anything remotely interesting, make a big deal about it!
    """,
    "assistant": """
        You are a helpful assistant and friendly companion.
    """
}

def encode_image(image_path):
    with open(image_path, "rb") as image_file:
        return base64.b64encode(image_file.read()).decode("utf-8")


# def text_from_audio(file_path): 
#     audio_file= open(file_path, "rb")
#     transcription = client.audio.transcriptions.create(
#         model="whisper-1", 
#         file=audio_file
#     )

#     print("Audio Transcript:")
#     print(transcription.text)

#     return transcription.text


def generate_new_line(base64_image):
    return [
        {
            "role": "user",
            "content": [
                {"type": "text", "text": "Describe this image. Pay special attention to the people in the photo."},
                {
                    "type": "image_url",
                    "image_url": f"data:image/jpeg;base64,{base64_image}",
                },
            ],
        },
    ]

def analyze_image(user, base64_image, command):
    system_prompt = personalities.get(user)
    system_prompt += "\n Pay extra attention to the details of the people in the photo and their surroundings when responding."
    
    system_message = {
        "role": "system",
        "content": system_prompt,
    }

    user_message = {
        "role": "user",
        "content": [
            {
                "type": "text", "text": command
            },
            {
                "type": "image_url",
                "image_url": f"data:image/jpeg;base64,{base64_image}",
            },
        ],
    }

    return execute_chat_completion(VISION_MODEL, system_message, user_message)   


def generate_response(user, command):
    system_prompt = personalities.get(user)
    system_message = {
        "role": "system",
        "content": system_prompt
    }

    user_message = {
        "role": "user",
        "content": command
    }

    return execute_chat_completion(DEFAULT_MODEL, system_message, user_message)


def execute_chat_completion(model, system_message, user_message, max_tokens=250):  
    global chat_history
    chat_history.append(user_message)
    full_chat = [system_message] + chat_history
    
    response = client.chat.completions.create(
        model=model,
        messages=full_chat,
        max_tokens=max_tokens
    )

    response_message = response.choices[0].message
    chat_history.append(response_message)

    if len(chat_history) > MAX_CHAT_HISTORY: 
        chat_history = chat_history[-MAX_CHAT_HISTORY]
        
    return response_message.content


def main():   
    # Getting the base64 encoding
    base64_image = encode_image("test.png")

    output = analyze_image(base64_image, [])
    print(output)

if __name__ == "__main__":    
    main()
