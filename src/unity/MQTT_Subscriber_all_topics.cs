using System;
using System.Text;
using UnityEngine;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;

public class MQTTSubscriber : MonoBehaviour
{
    private MqttClient client;
    private string brokerAddress = "192.168.137.252"; // Replace with your MQTT broker address
    private int brokerPort = 1883; // MQTT broker port
    private string mqttUser = "username"; // Replace with your MQTT username
    private string mqttPassword = "password"; // Replace with your MQTT password

    private string[] topics = new string[]
    {
        "esp8266/accel",
        "esp8266/gyro",
        "esp8266/ir",
        "esp8266/rotary",
        "esp8266/button"
    };

    private byte[] qosLevels = new byte[] { MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE };

    void Start()
    {
        ConnectToBroker();
    }

    void ConnectToBroker()
    {
        try
        {
            // Initialize MQTT client
            client = new MqttClient(brokerAddress, brokerPort, false, null, null, MqttSslProtocols.None);

            // Register event handlers
            client.MqttMsgPublishReceived += OnMessageReceived;

            // Connect to MQTT broker
            string clientId = Guid.NewGuid().ToString();
            client.Connect(clientId, mqttUser, mqttPassword);

            if (client.IsConnected)
            {
                Debug.Log("Connected to MQTT broker.");

                // Subscribe to topics
                foreach (string topic in topics)
                {
                    client.Subscribe(new string[] { topic }, new byte[] { MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE });
                }

                Debug.Log("Subscribed to topics.");
            }
            else
            {
                Debug.LogError("Failed to connect to MQTT broker.");
            }
        }
        catch (Exception ex)
        {
            Debug.LogError($"Connection error: {ex.Message}");
        }
    }

    void OnMessageReceived(object sender, MqttMsgPublishEventArgs e)
    {
        string topic = e.Topic;
        string payload = Encoding.UTF8.GetString(e.Message);
        Debug.Log($"Received message - Topic: {topic}, Message: {payload}");
    }

    void OnApplicationQuit()
    {
        if (client != null && client.IsConnected)
        {
            client.Disconnect();
            Debug.Log("Disconnected from MQTT broker.");
        }
    }
}

