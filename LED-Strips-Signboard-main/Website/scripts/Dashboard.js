const API_URL = 'http://127.0.0.1:8080'; // Change 127.0.0.1 to localhost if you receive post errors




async function sendMessage()
{
	let message = document.getElementById('message').value;
	let message2 = document.getElementById('message2').value;
	let animation = document.querySelector('input[name="animation"]:checked').value;
	let isBig = document.querySelector('input[name="isBig"]:checked').value;
    let send = document.getElementById("btn_send");

	if (isBig === "no")
	{
			message = message + "," + message2;
	}

	if (animation == "scroll")
	{
		animation = document.querySelector('input[name="scrollType"]:checked').value;
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
		
		// Send using new ASCII protocol format
		const response = await fetch(`${API_URL}/dashboard/post`,
		{
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(
			{
				"command": animation,
				"isBig": isBig,
				"data": message
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

async function send_settings()
{
	let set_btn = document.querySelector('.save-button');
	const brightness_value = document.getElementById('brightnessSlider').value;
	const top_color = document.getElementById('topTextcolour').value;
	const bottom_color = document.getElementById('bottomTextcolour').value;
    const full_text_color = document.getElementById('fullScreenTextcolour').value;
    set_btn.disabled = true;
	set_btn.style.cursor = "not-allowed";

	var actual_brightness_value = (brightness_value / 100) * 255; // Getting the percentage of 255. Because it is 0 - 255 on the arduino

	setTimeout(function()
	{
		set_btn.disabled = false;
		set_btn.style.cursor = "pointer";
	}, 3000);
	
	// Convert hex colors to RRGGBB format (remove #)
	const topColorHex = top_color.replace('#', '');
	const bottomColorHex = bottom_color.replace('#', '');
	const fullColorHex = full_text_color.replace('#', '');
	
	const response = await fetch(`${API_URL}/dashboard/post`,
	{
		method: 'POST',
		headers: {"Content-Type": "application/json"},
		body: JSON.stringify(
		{
			"command": "settns",
			"brightness": actual_brightness_value,
			"tcolor": topColorHex,
			"bcolor": bottomColorHex,
			"fcolor": fullColorHex,
		})
	});

	if (response.status != 200)
	{
		alert('Failed to send settings');
	}
	else
	{
		console.log("Settings sent using ASCII protocol");
	}
}

function handle_change(val)
{
	
	const slider = document.querySelector('.brightness-value');
	slider.innerHTML = val;
}