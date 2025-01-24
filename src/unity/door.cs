using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Door : MonoBehaviour
{
    public float interactionDistance = 3.0f;
    public GameObject intText;
    public string doorOpenAnimName = "DoorOpen";
    public string doorCloseAnimName = "DoorClose";
    public Transform player;
    private bool isPlayerInRange;

    protected KeyCode doorKey = KeyCode.None; // Default key

    private Animator doorAnim;
    private bool isOpen = false;

    private bool toggleDoorStatus = false;

    void Start()
    {
        doorAnim = GetComponent<Animator>();
        if (intText != null)
        {
            intText.SetActive(false); // Hide interaction text initially
        }

        // Subscribe to IR sensor events from MQTTManager
        if (MQTTSubscriber.Instance != null)
        {
            MQTTSubscriber.Instance.OnIRSensorTriggered += OnIRSensorTriggered;
        }
    }

    void Update()
    {

        CheckPlayerDistance();

        if (isPlayerInRange)
        {
                if (intText != null)
                {
                    intText.SetActive(false);  // Show interaction text when player is in range
                }

                // React to toggleDoorStatus updated by the MQTTSubscriber
                if (toggleDoorStatus)
                {
                    ToggleDoor();
                toggleDoorStatus = false; // Reset the status
                }
                }
         else
        {
            if (intText != null)
            {
                intText.SetActive(false);  // Hide interaction text when player is out of range
            }
        }
    }

    void CheckPlayerDistance()
    {
        if (player != null)
        {
            float distance = Vector3.Distance(player.position, transform.position);
            isPlayerInRange = distance <= interactionDistance;
        }
    }

    void ToggleDoor()
    {
        if (isOpen)
        {
            doorAnim.ResetTrigger("open");
            doorAnim.SetTrigger("close");
        }
        else
        {
            doorAnim.ResetTrigger("close");
            doorAnim.SetTrigger("open");
        }

        isOpen = !isOpen; // Toggle the door state
    }

    void OnIRSensorTriggered(bool objectDetected)
    {
        if (objectDetected)
        {
            Debug.Log("IR Sensor detected an object. Toggling door status.");
            toggleDoorStatus = true;
        }
    }

    void OnDestroy()
    {
        // Unsubscribe from MQTTSubscriber events
        if (MQTTSubscriber.Instance != null)
        {
            MQTTSubscriber.Instance.OnIRSensorTriggered -= OnIRSensorTriggered;
        }
    }
}