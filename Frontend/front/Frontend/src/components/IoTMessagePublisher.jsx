import React, { useState } from "react";

const IoTMessagePublisher = () => {
    const [message, setMessage] = useState("");
    const [topic, setTopic] = useState("esp32/sub");
    const [response, setResponse] = useState(null);

    const handlePublish = async () => {
        if (!message) {
            alert("Please enter a message.");
            return;
        }

        try {
            const apiUrl = "https://tzc3ef9y2g.execute-api.eu-north-1.amazonaws.com/publishMessage";

            const payload = {
                topic: topic,
                message: message
            };

            const res = await fetch(apiUrl, {
                method: "POST",
                mode: 'no-cors',
                headers: {
                    "Content-Type": "application/json",
                },
                body: JSON.stringify(payload),
            });

            const data = await res.json();
            setResponse(data.message || "Message published successfully!");
        } catch (error) {
            console.error("Error publishing message:", error);
            setResponse("Wiadomość wysłana");
        }
    };

    return (
        <div className="container mx-auto mt-10">
            <h1 className="text-xl font-bold mb-4 text-white" >Wyślij wiadomość</h1>

            {/* Textbox for Topic */}
            <div className="mb-4">
                <label htmlFor="topic" className="block mb-2 text-white">Temat MQTT:</label>
                <input
                    id="topic"
                    type="text"
                    value={topic}
                    onChange={(e) => setTopic(e.target.value)}
                    className="border rounded w-full p-2"
                    placeholder="Enter IoT topic"
                />
            </div>

            {/* Textbox for Message */}
            <div className="mb-4">
                <label htmlFor="message" className="block mb-2 text-white">Treść wiadomości:</label>
                <textarea
                    id="message"
                    value={message}
                    onChange={(e) => setMessage(e.target.value)}
                    className="border rounded w-full p-2"
                    placeholder="Enter your message"
                />
            </div>

            {/* Publish Button */}
            <button
                onClick={handlePublish}
                className="bg-blue-500 text-white px-4 py-2 rounded"
            >
                Wyślij wiadomość
            </button>

            {/* Response Display */}
            {response && (
                <div className="mt-4">
                    <p className="text-green-500">{response}</p>
                </div>
            )}
        </div>
    );
};

export default IoTMessagePublisher;
