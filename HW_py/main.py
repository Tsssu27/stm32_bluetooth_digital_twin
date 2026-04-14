import customtkinter as ctk
import serial
import serial.tools.list_ports
import threading
import re
import time

class STM32Controller(ctk.CTk):
    def __init__(self):
        super().__init__()

        # UI Setup: Dark Mode and Blue Theme
        ctk.set_appearance_mode("dark")
        ctk.set_default_color_theme("blue")

        self.title("STM32 Console")
        self.geometry("450x550")
        self.resizable(False, False)
        
        self.serial_port = None
        self.is_connected = False
        self.read_thread = None

        # --- Top Area: Connection ---
        self.top_frame = ctk.CTkFrame(self)
        self.top_frame.pack(pady=20, padx=20, fill="x")

        self.port_var = ctk.StringVar(value="Select Port")
        self.port_combobox = ctk.CTkComboBox(self.top_frame, variable=self.port_var, values=["Scanning..."])
        self.port_combobox.pack(side="left", padx=10, pady=10, expand=True, fill="x")

        self.connect_btn = ctk.CTkButton(self.top_frame, text="Connect", command=self.toggle_connection)
        self.connect_btn.pack(side="right", padx=10, pady=10)

        # --- Middle Area: Data Display ---
        self.mid_frame = ctk.CTkFrame(self)
        self.mid_frame.pack(pady=10, padx=20, fill="both", expand=True)

        # Fluorescent Green Color
        self.angle_label = ctk.CTkLabel(self.mid_frame, text="--°", font=("Helvetica", 60, "bold"), text_color="#00FFAA") 
        self.angle_label.pack(pady=(40, 20))

        self.progress_bar = ctk.CTkProgressBar(self.mid_frame, width=300)
        self.progress_bar.set(0)
        self.progress_bar.pack(pady=20)

        # --- Bottom Area: Control ---
        self.bottom_frame = ctk.CTkFrame(self)
        self.bottom_frame.pack(pady=20, padx=20, fill="x")

        self.slider_label = ctk.CTkLabel(self.bottom_frame, text="Target Angle: 90°", font=("Helvetica", 14))
        self.slider_label.pack(pady=(10, 0))

        self.slider = ctk.CTkSlider(self.bottom_frame, from_=0, to=180, number_of_steps=180, command=self.slider_event)
        self.slider.set(90)
        self.slider.pack(pady=20, padx=20, fill="x")
        self.slider.configure(state="disabled")

        # Regex for incoming angle
        self.angle_pattern = re.compile(r"ANGLE:(\d+)")
        
        # Save default colors to restore later
        self.btn_default_fg = self.connect_btn.cget("fg_color")
        self.btn_default_hover = self.connect_btn.cget("hover_color")

        # Start port refresh
        self.refresh_ports()
        
    def get_ports(self):
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports] if ports else ["No Ports Found"]

    def refresh_ports(self):
        if not self.is_connected:
            current_ports = self.get_ports()
            self.port_combobox.configure(values=current_ports)
            if self.port_var.get() not in current_ports and current_ports != ["No Ports Found"]:
                self.port_var.set(current_ports[0])
            elif current_ports == ["No Ports Found"]:
                self.port_var.set("No Ports Found")
        
        # Refresh every 2 seconds
        self.after(2000, self.refresh_ports)

    def toggle_connection(self):
        if not self.is_connected:
            port = self.port_var.get()
            if port in ("Select Port", "No Ports Found", "Scanning..."):
                return
            try:
                # Connected: 9600 baudrate for Bluetooth module ZS040
                self.serial_port = serial.Serial(port, 9600, timeout=1)
                self.is_connected = True
                
                # Update UI to Disconnect state
                self.connect_btn.configure(text="Disconnect", fg_color="#C62828", hover_color="#B71C1C")
                self.port_combobox.configure(state="disabled")
                self.slider.configure(state="normal")
                
                # Start reading thread (daemon thread prevents blocking main UI loop)
                self.read_thread = threading.Thread(target=self.serial_read_loop, daemon=True)
                self.read_thread.start()
                
            except Exception as e:
                print(f"Error connecting: {e}")
        else:
            self.is_connected = False
            if self.serial_port and self.serial_port.is_open:
                self.serial_port.close()
            
            # Revert UI directly to default Connect state
            self.connect_btn.configure(text="Connect", fg_color=self.btn_default_fg, hover_color=self.btn_default_hover)
            self.port_combobox.configure(state="normal")
            self.slider.configure(state="disabled")
            self.angle_label.configure(text="--°")
            self.progress_bar.set(0)

    def serial_read_loop(self):
        while self.is_connected and self.serial_port and self.serial_port.is_open:
            try:
                if self.serial_port.in_waiting:
                    line = self.serial_port.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        match = self.angle_pattern.search(line)
                        if match:
                            angle_str = match.group(1)
                            angle = int(angle_str)
                            # Safe dispatch to main UI loop
                            self.after(0, self.update_ui, angle)
            except Exception as e:
                print(f"Serial read error: {e}")
                self.after(0, self.connection_lost)
                break
            time.sleep(0.01)

    def connection_lost(self):
        if self.is_connected:
            self.toggle_connection()

    def update_ui(self, angle):
        if self.is_connected:
            # Bound 0-180
            angle = max(0, min(180, angle))
            self.angle_label.configure(text=f"{angle}°")
            self.progress_bar.set(angle / 180.0)

    def slider_event(self, value):
        angle = int(value)
        self.slider_label.configure(text=f"Target Angle: {angle}°")
        # Send via serial if connected
        if self.is_connected and self.serial_port and self.serial_port.is_open:
            cmd = f"<CMD:ANGLE:{angle}>\n"
            try:
                self.serial_port.write(cmd.encode('utf-8'))
            except Exception as e:
                print(f"Serial write error: {e}")

if __name__ == "__main__":
    app = STM32Controller()
    app.mainloop()
