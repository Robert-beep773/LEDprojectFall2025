/**
 * LED Strips Signboard - Dashboard JavaScript API
 * 
 * This file contains all API functions for communicating with the
 * PowerShell web server, which bridges HTTP requests to Arduino serial communication.
 * 
 * Communication Flow:
 * Browser → HTTP POST → PowerShell Server → Serial → Arduino
 * 
 * @author LED Strips Signboard Team
 * @version 2.0
 */

// API endpoint URL - Change to localhost if 127.0.0.1 causes issues
const API_URL = 'http://127.0.0.1:8080';

/**
 * Send text message to display
 * 
 * Collects user input from the Dashboard form and sends it to the server.
 * The server converts the JSON command to ASCII protocol format for Arduino.
 * 
 * Command mapping:
 * - static → 1001 (small) or 1002 (large)
 * - scrolC → 1003 (small) or 1004 (large) - Continuous scroll
 * - scrolS → 1005 (small) or 1006 (large) - Scroll and stop
 * - fadeIn → 1007 (small) or 1008 (large)
 * - breath → 1009 (small) or 1010 (large)
 * 
 * @async
 */
async function sendMessage()
{
	// Get form values
	let message = document.getElementById('message').value;
	let message2 = document.getElementById('message2').value;
	let animation = document.querySelector('input[name="animation"]:checked').value;
	let isBig = document.querySelector('input[name="isBig"]:checked').value;
    let send = document.getElementById("btn_send");

	if (isBig === "no")
	{
			message = message + "," + message2;
	}

	// If scroll animation selected, get the specific scroll type
	if (animation == "scroll")
	{
		animation = document.querySelector('input[name="scrollType"]:checked').value;
	}

	// Validate input
	if (!message)
	{
		alert('Please enter a message');
		return;
	}
	
	// Disable button to prevent rapid clicking
	send.disabled = true;
	send.style.cursor = "not-allowed";
	setTimeout(function(){
		send.disabled = false;
		send.style.cursor = "pointer";
	}, 3000);
	
	// Send JSON command to server
	// Server will convert to ASCII protocol: [START][COMMAND][DATA][END]
	const response = await fetch(`${API_URL}/dashboard/post`,
	{
		method: 'POST',
		headers: { 'Content-Type': 'application/json' },
		body: JSON.stringify(
		{
			"command": animation,  // Animation type
			"isBig": isBig,         // Font size: "yes" = large, "no" = small
			"data": message         // Text to display
		})
	});

	if (response.status != 200)
	{
		alert("Failed to connect");
	}
}

/**
 * Stop the PowerShell web server
 * 
 * Sends a kill command to gracefully shutdown the server.
 * This closes the serial port and stops the HTTP listener.
 * 
 * @async
 */
async function killServer()
{
	if (confirm('Are you sure you want to stop the server?'))
	{
		const response = await fetch(`${API_URL}/kill`)
		if (response.status === 200)
		{
			alert('Server stopped successfully');
		}
		else
		{
			alert("Connection to server failed");
		}
	}
}

/**
 * Start countdown timer
 * 
 * Validates input (0-99 minutes, 0-59 seconds) and sends timer start command.
 * Timer format: MM:SS (e.g., "05:30" for 5 minutes 30 seconds)
 * 
 * Command: sTimer → 2001 [MM:SS]
 * 
 * @async
 */
async function start_timer()
{
	const minutes = document.getElementById('minutes').value;
	const seconds = document.getElementById('seconds').value;
	let s_timer = document.querySelector('.btn-start');
	let sflag = true;

	// Validate minutes (0-99)
	if(parseInt(minutes) > 99)
	{
		alert("Max minutes allowed are 99");
		sflag = false;
	}
	
	// Validate seconds (0-59)
	if(parseInt(seconds) > 59)
	{
		alert("Max seconds are 59");
		sflag = false;
	}

    if(sflag)
	{
		// Format time as MM:SS for ASCII protocol
		const formattedMinutes = minutes.padStart(2, '0');
		const formattedSeconds = seconds.padStart(2, '0');
		const message = formattedMinutes + ":" + formattedSeconds;
		
		s_timer.disabled = true;
		s_timer.style.cursor = "not-allowed";
		setTimeout(function()
		{
			s_timer.disabled = false;
			s_timer.style.cursor = "pointer";
		}, 3000);
		
		const response = await fetch(`${API_URL}/dashboard/post`,
		{
			method: 'POST',
			headers: {"Content-Type": "application/json"},
			body: JSON.stringify(
			{
				"command": "sTimer",  // Maps to command 2001
				"isBig": "yes",        // Timer uses large font
				"data": message        // MM:SS format
			})
		});
	
		if (response.status != 200)
		{
			alert('Failed to send timer command');
		}
		else
		{
			console.log("Timer started using ASCII protocol: " + message);
		}
	}
}

async function pause_and_resume_timer()
{
	let p_timer = document.querySelector('.btn-pause-resume');
	let toggle = "";
	if(p_timer.textContent === "Pause")
	{
		toggle = "pTimer";
		p_timer.textContent = "Resume";
		p_timer.style.background = "green";
	}
	else
	{
		toggle = "resume";
		p_timer.textContent = "Pause";
		p_timer.style.background = "rgb(249, 142, 41)";
	}
	p_timer.disabled = true;
	p_timer.style.cursor = "not-allowed";
	setTimeout(function()
	{
		p_timer.disabled = false;
		p_timer.style.cursor = "pointer";
	}, 3000);
	const response = await fetch(`${API_URL}/dashboard/post`,
	{
		method: 'POST',
		headers: {"Content-Type": "application/json"},
		body: JSON.stringify(
		{
			"command": toggle
		})
	});

	if (response.status != 200)
	{
		alert('Failed to send message');
	}

}

async function display_time()
{
	let d_time = document.getElementById("timeOfDayBtn");
    d_time.disabled = true;
	d_time.style.cursor = "not-allowed";
	setTimeout(function(){
		d_time.disabled = false;
		d_time.style.cursor = "pointer";
	}, 3000);
	const response = await fetch(`${API_URL}/dashboard/post`,
	{
		method: 'POST',
		headers: {"Content-Type": "application/json"},
		body: JSON.stringify(
		{
			"command": "tod"
		})
	});

	if (response.status != 200)
	{
		alert('Failed to send message');
	}

	
}
async function reset_timer()
{
	let r_time = document.querySelector('.btn-reset');
    r_time.disabled = true;
	r_time.style.cursor = "not-allowed";
	setTimeout(function(){
		r_time.disabled = false;
		r_time.style.cursor = "pointer";
	}, 3000);
	const response = await fetch(`${API_URL}/dashboard/post`,
	{
		method: 'POST',
		headers: {"Content-Type": "application/json"},
		body: JSON.stringify(
		{
			"command": "rTimer"
		})
	});

	if (response.status != 200)
	{
		alert('Failed to send message');
	}
}

const all_preset_btns = document.querySelectorAll('#preset_btns');
all_preset_btns.forEach(btn => 
{
	btn.addEventListener('click', (e) => 
	{
		e.preventDefault();
		let time_val = btn.innerHTML;
		let data = time_val.split(':');
		document.getElementById('minutes').value = data[0];
	    document.getElementById('seconds').value = data[1];
	});
});

/**
 * Send brightness setting to Arduino
 * 
 * Converts slider value (0-100%) to Arduino brightness (0-255).
 * 
 * Command: settns → 3005 [brightness,topColor,bottomColor,fullColor]
 * 
 * @async
 */
async function send_brightness()
{
	let brightness_btn = document.getElementById('saveBrightness');
	const brightness_value = document.getElementById('brightnessSlider').value;
	
	// Disable button to prevent rapid clicking
	brightness_btn.disabled = true;
	brightness_btn.style.cursor = "not-allowed";

	// Convert percentage (0-100) to Arduino brightness value (0-255)
	// Arduino NeoPixel library uses 0-255 for brightness
	var actual_brightness_value = Math.round((brightness_value / 100) * 255);

	setTimeout(function()
	{
		brightness_btn.disabled = false;
		brightness_btn.style.cursor = "pointer";
	}, 3000);
	
	const response = await fetch(`${API_URL}/dashboard/post`,
	{
		method: 'POST',
		headers: {"Content-Type": "application/json"},
		body: JSON.stringify(
		{
			"command": "settns",
			"brightness": actual_brightness_value,
			"tcolor": top_color,
			"bcolor": bottom_color,
			"fcolor": full_text_color,
		})
	});

	if (response.status != 200)
	{
		alert('Failed to send message');
	}
}

/**
 * Send color settings to Arduino
 * 
 * Converts hex color values (#RRGGBB) to RRGGBB format (no #).
 * Sends top text color, bottom text color, and full screen text color.
 * 
 * Command: settns → 3005 [brightness,topColor,bottomColor,fullColor]
 * Color format: RRGGBB (e.g., "FF0000" for red)
 * 
 * @async
 */
async function send_colors()
{
	let colors_btn = document.getElementById('saveColors');
	const top_color = document.getElementById('topTextcolour').value;
	const bottom_color = document.getElementById('bottomTextcolour').value;
    const full_text_color = document.getElementById('fullScreenTextcolour').value;
    
	// Disable button to prevent rapid clicking
	colors_btn.disabled = true;
	colors_btn.style.cursor = "not-allowed";

	setTimeout(function()
	{
		colors_btn.disabled = false;
		colors_btn.style.cursor = "pointer";
	}, 3000);
	
	// Convert hex colors to RRGGBB format (remove # symbol)
	// Only send colors if they are valid (not empty)
	// Empty string means "no change" for that color
	const topColorHex = top_color && top_color.length > 0 ? top_color.replace('#', '') : '';
	const bottomColorHex = bottom_color && bottom_color.length > 0 ? bottom_color.replace('#', '') : '';
	const fullColorHex = full_text_color && full_text_color.length > 0 ? full_text_color.replace('#', '') : '';
	
	// Send settings command (only colors, brightness empty)
	const response = await fetch(`${API_URL}/dashboard/post`,
	{
		method: 'POST',
		headers: {"Content-Type": "application/json"},
		body: JSON.stringify(
		{
			"command": "settns",      // Maps to command 3005
			"brightness": "",          // Empty = no change
			"tcolor": topColorHex,    // Top text color (RRGGBB)
			"bcolor": bottomColorHex,  // Bottom text color (RRGGBB)
			"fcolor": fullColorHex,    // Full screen text color (RRGGBB)
		})
	});

	if (response.status != 200)
	{
		alert('Failed to send color settings');
	}
	else
	{
		console.log("Color settings sent using ASCII protocol");
	}
}

function handle_brightness_change(val)
{
	const slider = document.querySelector('.brightness-value');
	slider.innerHTML = val + "%";
}

function update_color_value(colorId, value)
{
	const valueElement = document.getElementById(colorId + 'Value');
	if (valueElement) {
		valueElement.textContent = value.toUpperCase();
	}
}