using System.Net.Sockets;
using UnityEngine;
using System.Text;

public class NodeMCUCommunication : MonoBehaviour
{
    TcpClient client;
    NetworkStream stream;
    byte[] buffer = new byte[1024]; // Pre-allocated buffer to store incoming data

    void Start()
    {
        try
        {
            // Connect to the ESP8266 at its IP address and port 80
            client = new TcpClient("192.168.4.1", 80); // Replace with NodeMCU's IP
            stream = client.GetStream();
            Debug.Log("Connected to NodeMCU.");
        }
        catch (SocketException e)
        {
            Debug.LogError("Failed to connect to NodeMCU: " + e.Message);
        }
    }

    void Update()
    {
        // Continuously check if there's data available
        if (client != null && stream != null && stream.DataAvailable)
        {
            int bytesRead = stream.Read(buffer, 0, buffer.Length); // Read data into buffer
            if (bytesRead > 0)
            {
                string message = Encoding.UTF8.GetString(buffer, 0, bytesRead);
                Debug.Log("Message from NodeMCU: " + message); // Log the received message
            }
        }
    }

    void OnDestroy()
    {
        // Clean up resources
        if (stream != null) stream.Close();
        if (client != null) client.Close();
        Debug.Log("Disconnected from NodeMCU.");
    }
}
