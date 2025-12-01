const API_URL = 'http://127.0.0.1:8080'; // Change 127.0.0.1 to localhost if you receive post errors




async function sendMessage()
{
	let message = document.getElementById('message').value;
	let message2 = document.getElementById('message2').value;
	let animation = document.querySelector('input[name="animation"]:checked').value;
	let isBig = document.querySelector('input[name="isBig"]:checked').value;
    let send = document.getElementById("btn_send");

	// Get scroll type if animation is scroll
	if (animation == "scroll")
	{
		animation = document.querySelector('input[name="scrollType"]:checked').value;
	}

	// Validate character limits (only for static and scroll then stop)
	// Scroll continuous allows up to 120 characters
	if (animation === "static" || animation === "scrolS") {
		if (isBig === "yes") {
			// Full screen: max 4 characters
			if (message.length > 4) {
				alert('Full screen text can only fit 4 characters');
				return;
			}
		} else {
			// Top/Bottom: max 10 characters each
			if (message.length > 10) {
				alert('Top text can only fit 10 characters');
				return;
			}
			if (message2.length > 10) {
				alert('Bottom text can only fit 10 characters');
				return;
			}
		}
	} else if (animation === "scrolC") {
		// Scroll continuous: allow up to 120 characters total
		let totalLength = isBig === "no" ? message.length + message2.length + 1 : message.length; // +1 for comma
		if (totalLength > 120) {
			alert('Scroll continuous text can only fit 120 characters total');
			return;
		}
	}

	if (isBig === "no")
	{
			message = message + "," + message2;
	}

	if (!message)
	{
		alert('Please enter a message');
	}
	else
	{
		send.disabled = true;
		send.style.cursor = "not-allowed";
		setTimeout(function(){
			send.disabled = false;
			send.style.cursor = "pointer";
		}, 3000);
		
		// Get scroll speed if animation is scroll
		let scrollSpeed = "fast"; // default
		if (animation === "scrolC" || animation === "scrolS") {
			scrollSpeed = document.querySelector('input[name="scrollSpeed"]:checked').value;
		}
		
		// Send using new ASCII protocol format
		const response = await fetch(`${API_URL}/dashboard/post`,
		{
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(
			{
				"command": animation,
				"isBig": isBig,
				"data": message,
				"scrollSpeed": scrollSpeed
			})
		});
	
		if (response.status != 200)
		{
			alert("Failed to connect");
		}
		else
		{
			console.log("Message sent successfully using ASCII protocol");
		}
	}
}

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

async function start_timer()
{
	const minutes = document.getElementById('minutes').value;
	const seconds = document.getElementById('seconds').value;
	let s_timer = document.querySelector('.btn-start');
	let sflag = true;

	if(parseInt(minutes) > 99)
	{
		alert("Max minutes allowed are 99");
		sflag = false;
	}
	
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
				"command": "sTimer",
				"isBig": "yes",
				"data": message
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

async function send_brightness()
{
	let brightness_btn = document.getElementById('saveBrightness');
	const brightness_value = document.getElementById('brightnessSlider').value;
	
	brightness_btn.disabled = true;
	brightness_btn.style.cursor = "not-allowed";

	var actual_brightness_value = Math.round((brightness_value / 100) * 255); // Getting the percentage of 255. Because it is 0 - 255 on the arduino

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
			"tcolor": "",
			"bcolor": "",
			"fcolor": "",
		})
	});

	if (response.status != 200)
	{
		alert('Failed to send brightness settings');
	}
	else
	{
		console.log("Brightness settings sent using ASCII protocol");
	}
}

async function send_colors()
{
	let colors_btn = document.getElementById('saveColors');
	const top_color = document.getElementById('topTextcolour').value;
	const bottom_color = document.getElementById('bottomTextcolour').value;
    const full_text_color = document.getElementById('fullScreenTextcolour').value;

	colors_btn.disabled = true;
	colors_btn.style.cursor = "not-allowed";

	setTimeout(function()
	{
		colors_btn.disabled = false;
		colors_btn.style.cursor = "pointer";
	}, 3000);
	
	// Convert hex colors to RRGGBB format (remove #)
	// Only send colors if they are valid (not empty)
	const topColorHex = top_color && top_color.length > 0 ? top_color.replace('#', '') : '';
	const bottomColorHex = bottom_color && bottom_color.length > 0 ? bottom_color.replace('#', '') : '';
	const fullColorHex = full_text_color && full_text_color.length > 0 ? full_text_color.replace('#', '') : '';
	
	const response = await fetch(`${API_URL}/dashboard/post`,
	{
		method: 'POST',
		headers: {"Content-Type": "application/json"},
		body: JSON.stringify(
		{
			"command": "settns",
			"brightness": "",
			"tcolor": topColorHex,
			"bcolor": bottomColorHex,
			"fcolor": fullColorHex,
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