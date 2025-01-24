
using System;
using System.Text;
using UnityEngine;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;

public class MQTTSubscriber : MonoBehaviour
{
    private MqttClient client;
    public string brokerAddress = "192.168.137.252";
    public int brokerPort = 1883;
    public string mqttUser = "username";
    public string mqttPassword = "password";

    public static MQTTSubscriber Instance; // Singleton instance

    public event Action<bool> OnIRSensorTriggered; // Event to notify Door script

    void Awake()
    {
        if (Instance == null)
        {
            Instance = this;
            DontDestroyOnLoad(gameObject); // Keep MQTTSubscriber alive across scenes
        }
        else
        {
            Destroy(gameObject);
        }
    }

    void Start()
    {
        ConnectToBroker();
    }

    void ConnectToBroker()
    {
        try
        {
            client = new MqttClient(brokerAddress, brokerPort, false, null, null, MqttSslProtocols.None);
            client.MqttMsgPublishReceived += OnMessageReceived;
            string clientId = Guid.NewGuid().ToString();
            client.Connect(clientId, mqttUser, mqttPassword);

            if (client.IsConnected)
            {
                Debug.Log("Connected to MQTT broker.");
                client.Subscribe(new string[] { "esp8266/ir" }, new byte[] { MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE });
            }
        }
        catch (Exception ex)
        {
            Debug.LogError($"MQTT connection error: {ex.Message}");
        }
    }

    void OnMessageReceived(object sender, MqttMsgPublishEventArgs e)
    {
        string topic = e.Topic;
        string payload = Encoding.UTF8.GetString(e.Message);

        Debug.Log($"MQTT Message Received - Topic: {topic}, Payload: {payload}");

        if (topic == "esp8266/ir")
        {
            bool objectDetected = payload == "Object Detected";
            OnIRSensorTriggered?.Invoke(objectDetected); // Notify listeners
        }
    }

    void OnApplicationQuit()
    {
        if (client != null && client.IsConnected)
        {
            client.Disconnect();
        }
    }
}
