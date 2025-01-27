using TMPro;
using UnityEngine;

public class TemperatureDisplay : MonoBehaviour
{
    public float temperature = 20.0f; // Default temperature value
    private TextMeshPro textMeshPro;

    void Start()
    {
        // Get the TextMeshPro component attached to this GameObject
        textMeshPro = GetComponent<TextMeshPro>();

        // Initialize the display with the default temperature
        UpdateTemperatureDisplay();

        // Subscribe to temperature updates from MQTTSubscriber
        if (MQTTSubscriber.Instance != null)
        {
            MQTTSubscriber.Instance.OnTemperatureUpdated += HandleTemperatureUpdate;
        }
    }

    void OnDestroy()
    {
        // Unsubscribe from the temperature update event to prevent memory leaks
        if (MQTTSubscriber.Instance != null)
        {
            MQTTSubscriber.Instance.OnTemperatureUpdated -= HandleTemperatureUpdate;
        }
    }

    void HandleTemperatureUpdate(float newTemperature)
    {
        // Update the temperature value when notified by MQTTSubscriber
        temperature = newTemperature;
    }

    void Update()
    {
        // Continuously update the display with the current temperature
        UpdateTemperatureDisplay();
    }

    public void UpdateTemperatureDisplay()
    {
        // Check if the TextMeshPro component is assigned before updating the text
        if (textMeshPro != null)
        {
            // Update the text with the current temperature value
            textMeshPro.text = temperature.ToString("F1") + " °C";
        }
    }
}