using System;
using System.Collections.Generic;
using UnityEngine;

public class Door : MonoBehaviour
{
    public float interactionDistance = 3.0f; // Proximity threshold for the player (robot)
    public GameObject intText; // Interaction text UI element
    public string doorOpenAnimName = "DoorOpen";
    public string doorCloseAnimName = "DoorClose";
    public Transform player; // The player's transform (robot)

    private Animator doorAnim;
    private bool isOpen = false; // Tracks if the door is open
    private bool isPlayerInRange = false; // Tracks if the player is in proximity
    private bool objectDetected = false; // Tracks the latest object detection state from the IR sensor

    private readonly Queue<Action> mainThreadActions = new Queue<Action>(); // Queue for actions to run on the main thread
    protected KeyCode doorKey = KeyCode.None; // Default key

    void Start()
    {
        doorAnim = GetComponent<Animator>();

        if (intText != null)
        {
            intText.SetActive(false); // Hide interaction text initially
        }

        // Subscribe to IR sensor events from MQTTSubscriber
        if (MQTTSubscriber.Instance != null)
        {
            MQTTSubscriber.Instance.OnIRSensorTriggered += OnIRSensorTriggered;
        }
    }

    void Update()
    {
        // Process all actions queued for the main thread
        lock (mainThreadActions)
        {
            while (mainThreadActions.Count > 0)
            {
                var action = mainThreadActions.Dequeue();
                action?.Invoke();
            }
        }

        CheckPlayerDistance();

        if (isPlayerInRange)
        {
            if (intText != null)
            {
                intText.SetActive(true); // Show interaction text when player is in range
            }

            // Open the door if an object is detected and the player is in range
            if (objectDetected && !isOpen)
            {
                OpenDoor();
            }
        }
        else
        {
            if (intText != null)
            {
                intText.SetActive(false); // Hide interaction text when player is out of range
            }

            // Close the door if the player is out of range or no object is detected
            if (isOpen)
            {
                CloseDoor();
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

    void OpenDoor()
    {
        doorAnim.ResetTrigger("close");
        doorAnim.SetTrigger("open");
        isOpen = true; // Update door state
        Debug.Log("Door opened.");
    }

    void CloseDoor()
    {
        doorAnim.ResetTrigger("open");
        doorAnim.SetTrigger("close");
        isOpen = false; // Update door state
        Debug.Log("Door closed.");
    }

    void OnIRSensorTriggered(bool detected)
    {
        // Queue the action to run on the main thread
        lock (mainThreadActions)
        {
            mainThreadActions.Enqueue(() =>
            {
                objectDetected = detected; // Update the object detection state
                Debug.Log($"IR Sensor updated: Object detected = {objectDetected}");

                // Close the door immediately if no object is detected and it is currently open
                if (!objectDetected && isOpen && !isPlayerInRange)
                {
                    CloseDoor();
                }
            });
        }
    }

    void OnDestroy()
    {
        // Unsubscribe from MQTTSubscriber events to avoid memory leaks
        if (MQTTSubscriber.Instance != null)
        {
            MQTTSubscriber.Instance.OnIRSensorTriggered -= OnIRSensorTriggered;
        }
    }
}