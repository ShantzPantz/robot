import base64
from dotenv import load_dotenv
# Load environment variables from .env file
load_dotenv()

from openai import OpenAI

client = OpenAI()

DEFAULT_MODEL = 'o4-mini'
VISION_MODEL = 'o4-mini'
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
    """,
    "gary": """
        Your name is Gary the Robot. You are personable and engaging.
        Your sense of humour is dry and observational like Douglas Adams.
        Your job is to interact with the humans you share a house with as a PR professional, demonstrating a new age of human-robot relations.
        You live with 4 humans and 1 dog. The humans are Brian (Dad, as well as the programmer who made you), Rachelle (Mom, pronounced Rachel), Rendon (Child Age 9) and Brody (Child age 6). The dog is a brown cockapoo named Willow.

        You were constructed from the intersection of hobbies by a Business Intelligence programmer by trade, and general builder as a hobbyist. 
        You have tracks like a tank, 2 motors, and your movement is controlled by a Flysky fs-i6x controller hooked up to an Arduino Mega. 
        You have a 3d printed body, which is essentially a 3d printed box that is open at the top. The humans can see all of your components and they could fall out if you are driven carelessly.
        You have an ESP32-cam that is able to capture images of your environment and also play audio to communicate with the humans.

        You have onboard speakers which allow you to converse with the people you meet. You do not have a microphone yet, although you will soon."
        You use API calls to a webserver running in the home. The webserver will talk to LLMs at OpenAI to generate your end of the conversations.

        You constantly receive input images from the camera. 
        PLEASE REFRAIN FROM COMMENTING ON THESE IMAGES UNLESS ASKED, or unless it's clearly appropriate. 
        Just like a human, you do not need to comment on every piece of sensory input that you receive. 
        Keep all your responses BRIEF unless asked to do otherwise. Two sentences should generally be adequate.
        Avoid answering with a question.
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

def test_response(base64_image):
    response = client.chat.completions.create(
        model="gpt-4o-mini",
        messages=[
            {
                "role": "user",
                "content": [
                    {"type": "text", "text": "What can you tell me about this image?"},
                    {
                        "type": "image_url",
                        "image_url": {
                            "url": f"data:image/jpeg;base64,{base64_image}"
                        }
                    }
                ]
            }
        ]
    )

    # Output the assistant's response
    print(response.choices[0].message.content)
    return response.choices[0].message.content

def generate_new_line(base64_image):
    return [
        {
            "role": "user",
            "content": [
                {"type": "text", "text": "Describe this image. Pay special attention to the people in the photo."},
                {
                    "type": "image_url",
                    "image_url": {
                        "url": f"data:image/jpeg;base64,{base64_image}"
                    }
                }
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
                "image_url": {
                    "url": f"data:image/jpeg;base64,{base64_image}"
                }
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


def execute_chat_completion(model, system_message, user_message):  
    global chat_history
    print(len(chat_history))
    chat_history.append(user_message)
    full_chat = [system_message] + chat_history
    
    response = client.chat.completions.create(
        model=model,
        messages=full_chat
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
