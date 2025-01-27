using System;
using System.Text;
using System.Text.RegularExpressions; // For regular expressions
using UnityEngine;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;

public class MQTTSubscriber : MonoBehaviour
{
    private MqttClient client;

    // MQTT Broker Details
    public string brokerAddress = "192.168.137.252"; // Replace with your broker's IP
    public int brokerPort = 1883;
    public string mqttUser = "username"; // Optional MQTT user
    public string mqttPassword = "password"; // Optional MQTT password

    public static MQTTSubscriber Instance; // Singleton instance for global access

    // Events for subscribed topics
    public event Action<bool> OnIRSensorTriggered; // Event for IR sensor
    public event Action<float> OnTemperatureUpdated; // Event for temperature
    public event Action<float, float> OnGyroscopeUpdated; // Event for gyroscope data (X, Y)
    public event Action<string> OnAccelerometerUpdated; // Event for accelerometer data
    public event Action<bool> OnCapacitiveSensorTriggered; // Event for capacitive sensor
    public event Action<float> OnGyroscopeZaxisUpdated; // Event for camera rotation
    public event Action<bool> OnRunningTriggered; // Event for capacitive sensor

    // Gyroscope Data
    public float GyroInclinationX { get; private set; } = 0.0f; // Forward/backward tilt
    public float GyroInclinationY { get; private set; } = 0.0f; // Left/right tilt

    // Sensor States
    private bool previousIRState = false; // Tracks the last known state of the IR sensor
    private bool previousCapState = false; // Tracks the last known state of the capacitive sensor
    public float GyroscopeZ { get; private set; } = 0.0f;
    private bool previousRunState = false; // Tracks the last known state of the capacitive sensor

    void Awake()
    {
        // Singleton pattern: ensure only one instance of this object exists
        if (Instance == null)
        {
            Instance = this;
            DontDestroyOnLoad(gameObject); // Persist MQTTSubscriber across scenes
        }
        else
        {
            Debug.LogError("Duplicate MQTTSubscriber instances detected! Destroying this instance.");
            Destroy(gameObject);
        }
    }

    void Start()
    {
        ConnectToBroker();
    }

    private void ConnectToBroker()
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

                // Subscribe to relevant topics
                client.Subscribe(new string[]
                {
                    "esp8266/ir", "esp8266/temp", "esp8266/gyro", "esp8266/accel", "esp8266/touch", "esp8266/cam", "esp8266/run"
                }, new byte[]
                {
                    MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE,
                    MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE,
                    MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE,
                    MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE,
                    MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE,
                    MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE,
                    MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE
                });
            }
            else
            {
                Debug.LogError("Failed to connect to the MQTT broker.");
            }
        }
        catch (Exception ex)
        {
            Debug.LogError($"MQTT connection error: {ex.Message}");
        }
    }

    private void OnMessageReceived(object sender, MqttMsgPublishEventArgs e)
    {
        string topic = e.Topic;
        string payload = Encoding.UTF8.GetString(e.Message);

        Debug.Log($"MQTT Message Received - Topic: {topic}, Payload: {payload}");

        switch (topic)
        {
            case "esp8266/ir":
                HandleIRSensorData(payload);
                break;

            case "esp8266/temp":
                HandleTemperatureData(payload);
                break;

            case "esp8266/gyro":
                HandleGyroscopeData(payload);
                break;

            case "esp8266/accel":
                OnAccelerometerUpdated?.Invoke(payload); // Pass raw accelerometer data
                break;

            case "esp8266/touch":
                HandleCapacitiveSensorData(payload);
                break;

            case "esp8266/cam": // Gyroscope Z-axis data for camera
                HandleGyroscopeZaxisData(payload);
                break;

            case "esp8266/run":
                HandleRunningSensorData(payload);
                break;

            default:
                Debug.LogWarning($"Unhandled MQTT topic: {topic}");
                break;
        }
    }

    private void HandleIRSensorData(string payload)
    {
        bool objectDetected = payload.Equals("Object Detected", StringComparison.OrdinalIgnoreCase);

        if (objectDetected != previousIRState)
        {
            previousIRState = objectDetected; // Update previous state
            OnIRSensorTriggered?.Invoke(objectDetected); // Notify listeners
            Debug.Log($"IR Sensor State Changed: {objectDetected}");
        }
    }

    private void HandleTemperatureData(string payload)
    {
        Match match = Regex.Match(payload, @"([-+]?[0-9]*\.?[0-9]+)");
        if (match.Success)
        {
            if (float.TryParse(match.Value, out float temperature))
            {
                OnTemperatureUpdated?.Invoke(temperature); // Notify listeners
                Debug.Log($"Temperature Updated: {temperature}°C");
            }
            else
            {
                Debug.LogError($"Failed to parse temperature value from payload: {payload}");
            }
        }
        else
        {
            Debug.LogError($"No valid temperature value found in payload: {payload}");
        }
    }

    private void HandleGyroscopeData(string payload)
    {
        try
        {
            string[] gyroData = payload.Split(',');
            if (gyroData.Length == 2)
            {
                if (float.TryParse(gyroData[0], System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out float parsedX))
                {
                    GyroInclinationX = parsedX;
                }
                else
                {
                    Debug.LogError($"Failed to parse X value: {gyroData[0]}");
                }

                if (float.TryParse(gyroData[1], System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out float parsedY))
                {
                    GyroInclinationY = parsedY;
                }
                else
                {
                    Debug.LogError($"Failed to parse Y value: {gyroData[1]}");
                }

                OnGyroscopeUpdated?.Invoke(GyroInclinationX, GyroInclinationY); // Notify listeners
                Debug.Log($"Gyroscope Updated: X={GyroInclinationX}, Y={GyroInclinationY}");
            }
            else
            {
                Debug.LogError($"Invalid gyroscope payload: {payload}");
            }
        }
        catch (Exception ex)
        {
            Debug.LogError($"Error parsing gyroscope data: {ex.Message}");
        }
    }

    private void HandleCapacitiveSensorData(string payload)
    {
        bool capSensorTriggered = payload.Equals("True", StringComparison.OrdinalIgnoreCase);

        if (capSensorTriggered != previousCapState)
        {
            previousCapState = capSensorTriggered; // Update previous state
            OnCapacitiveSensorTriggered?.Invoke(capSensorTriggered); // Notify listeners
            Debug.Log($"Capacitive Sensor State Changed: {capSensorTriggered}");
        }
    }

    private void HandleRunningSensorData(string payload)
    {
        bool runningTriggered = payload.Equals("True", StringComparison.OrdinalIgnoreCase);

        if (runningTriggered != previousRunState)
        {
            previousRunState = runningTriggered; // Update previous state
            OnRunningTriggered?.Invoke(runningTriggered); // Notify listeners
            Debug.Log($"Running Sensor State Changed: {runningTriggered}");
        }
    }

    private void HandleGyroscopeZaxisData(string payload)
    {
        if (float.TryParse(payload, System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out float gyroscopeZ))
        {
            GyroscopeZ = gyroscopeZ; // Update the Gyroscope Z value
            OnGyroscopeZaxisUpdated?.Invoke(gyroscopeZ); // Notify listeners
            Debug.Log($"Gyroscope Z-axis value received: {gyroscopeZ}");
        }
        else
        {
            Debug.LogError($"Failed to parse gyroscope Z-axis data: {payload}");
        }
    }



    private void OnApplicationQuit()
    {
        if (client != null && client.IsConnected)
        {
            client.Disconnect();
            Debug.Log("Disconnected from MQTT broker.");
        }
    }
}