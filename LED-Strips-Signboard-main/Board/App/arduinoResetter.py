import tkinter as tk
from tkinter import ttk, messagebox
import serial
import serial.tools.list_ports
import time

class ArduinoResetApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Arduino Reset Tool")
        self.root.geometry("400x200")
        self.root.resizable(False, False)
        
        # Create main frame
        main_frame = ttk.Frame(root, padding="20")
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # Port selection
        port_frame = ttk.Frame(main_frame)
        port_frame.pack(fill=tk.X, pady=10)
        
        ttk.Label(port_frame, text="Select Arduino Port:").pack(side=tk.LEFT, padx=5)
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(port_frame, textvariable=self.port_var, width=30)
        self.port_combo.pack(side=tk.LEFT, padx=5)
        
        # Refresh ports button
        refresh_btn = ttk.Button(main_frame, text="Refresh Ports", command=self.refresh_ports)
        refresh_btn.pack(pady=10)
        
        # Reset button
        reset_btn = ttk.Button(main_frame, text="RESET ARDUINO", 
                              command=self.reset_arduino, style="Reset.TButton")
        reset_btn.pack(pady=10, ipadx=20, ipady=10)
        
        # Create custom style for the reset button
        style = ttk.Style()
        style.configure("Reset.TButton", font=("Arial", 12, "bold"))
        
        # Status label
        self.status_var = tk.StringVar(value="Ready")
        status_label = ttk.Label(main_frame, textvariable=self.status_var)
        status_label.pack(pady=10)
        
        # Initialize port list
        self.refresh_ports()
    
    def refresh_ports(self):
        # Refresh the list of available serial ports
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_combo['values'] = ports
        if ports and not self.port_var.get():
            self.port_var.set(ports[0])
    
    def reset_arduino(self):
        # Send reset command to Arduino
        port = self.port_var.get()
        if not port:
            messagebox.showwarning("Port Selection", "Please select a port")
            return

        try:
            # Open connection
            self.status_var.set(f"Connecting to {port}...")
            self.root.update()

            # Open and close the port at 1200 baud - this triggers a reset on many Arduino boards
            ser = serial.Serial(port, 1200)
            time.sleep(0.1)
            ser.close()

            # Alternative reset approach - send "RESET" command
            time.sleep(0.5)
            ser = serial.Serial(port, 9600, timeout=1)
            time.sleep(0.1)
            ser.write(b"RESET\n")
            ser.close()

            self.status_var.set("Reset command sent successfully")
            self.root.update()
            self.root.destroy()  # <-- 0_- This line closes the window/app (remove if it shouldn't be a feature, it's not really needed to honest, idk)

        except Exception as e:
            self.status_var.set(f"Error: {str(e)}")
            messagebox.showerror("Reset Error", f"Failed to reset Arduino, close the server if it's open. Error message: {str(e)}")

if __name__ == "__main__":
    root = tk.Tk()
    app = ArduinoResetApp(root)
    root.mainloop()