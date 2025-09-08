import serial
import time
import threading

# Change this to your Arduino's port
PORT = "/dev/ttyUSB0"   # For Linux (check using `ls /dev/ttyUSB*`)
BAUD_RATE = 115200        # Must match Arduino's baud rate

# Open serial connection
ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # Wait for Arduino to reset after opening serial

print("Connected to Arduino on", PORT)
print("Press Ctrl+C to stop\n")

# Values to send repeatedly
values_to_send = ["L", "B"]
def send_values():
    """Send values to Arduino every 2 seconds."""
    try:
        for value in values_to_send:
            message = value + "\n"  # Always append newline
            ser.write(message.encode())
            print(f"Sent: {value} at {time.strftime('%H:%M:%S')}")
        # Schedule the next call after 2 seconds
        threading.Timer(2, send_values).start()
    except serial.SerialException:
        print("Error: Serial connection lost.")
    except Exception as e:
        print(f"Unexpected error: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Serial connection closed.")
