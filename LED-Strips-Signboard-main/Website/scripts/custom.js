document.addEventListener('DOMContentLoaded', function() 
{
    const pixelBoard = document.getElementById('pixelBoard');
    const colorPicker = document.getElementById('colorPicker');
    const eraserBtn = document.getElementById('eraserBtn');
    const clearBtn = document.getElementById('clearBtn');
    const sendBtn = document.getElementById('sendBtn');
    
    let currentColor = '#ffffff';
    let isDrawing = false;
    let isErasing = false;
    let drawnPixels = new Set();

    // Prevent default drag and selection behaviors
    pixelBoard.addEventListener('dragstart', (e) => e.preventDefault());
    pixelBoard.style.userSelect = 'none';
    pixelBoard.style.webkitUserSelect = 'none';
    
    // Create pixel board
    function createPixelBoard()
    {
        pixelBoard.innerHTML = '';
        
        for (let row = 0; row < 15; row++)
        {
            for (let col = 0; col < 60; col++)
            {
                const pixel = document.createElement('div');
                pixel.className = 'pixel';
                pixel.dataset.row = row;
                pixel.dataset.col = col;
                pixel.dataset.color = currentColor;
            
                pixel.style.backgroundColor = '#000000';
                
                // Prevent default drag behavior on each pixel
                pixel.addEventListener('mousedown', (e) => {
                    e.preventDefault();
                    startDrawing(e);
                });
                
                pixel.addEventListener('mouseover', draw);
                pixel.addEventListener('touchstart', handleTouch);
                pixel.addEventListener('touchmove', handleTouchMove);
                
                pixelBoard.appendChild(pixel);
            }
        }
        
        // Stop drawing when mouse leaves the board or mouse button is released
        document.addEventListener('mouseup', stopDrawing);
        document.addEventListener('mouseleave', stopDrawing);
        document.addEventListener('touchend', stopDrawing);
    }
    
    function startDrawing(e)
    {
        // Prevent default behavior and ensure it's left mouse button
        e.preventDefault();
        if (e.type === 'mousedown' && e.button !== 0) return;
        
        isDrawing = true;
        draw(e);
    }
    
    function draw(e)
    {
        // Only draw if mouse button is held down
        if (!isDrawing)
            return;
        
        let pixel = e.target;
        
        // Ensure we're drawing on a pixel
        if (!pixel.classList.contains('pixel')) return;
        
        let row = pixel.dataset.row;
        let col = pixel.dataset.col;
        let prevColor = pixel.style.backgroundColor;
        
        if (isErasing)
        {
            pixel.style.backgroundColor = '#000000';
            drawnPixels.delete(`(${row},${col},${rgbToHex(prevColor)})`);
        }
        else
        {
            pixel.style.backgroundColor = currentColor;
            drawnPixels.add(`(${row},${col},${currentColor})`);
        }
    }
    
    function stopDrawing()
    {
        isDrawing = false;
    }
    
    function handleTouch(e)
    {
        e.preventDefault();
        startDrawing(e);
    }
    
    function handleTouchMove(e)
    {
        e.preventDefault();
        const touch = e.touches[0];
        const pixel = document.elementFromPoint(touch.clientX, touch.clientY);
        
        if (pixel && pixel.classList.contains('pixel'))
        {
            let row = pixel.dataset.row;
            let col = pixel.dataset.col;
            let color = pixel.dataset.color;
            let prevColor = pixel.style.backgroundColor;
            
            if (isErasing)
            {
                pixel.style.backgroundColor = '#000000';
                drawnPixels.delete(`(${row},${col},${rgbToHex(prevColor)})`);
            }
            else
            {
                pixel.style.backgroundColor = currentColor;
                drawnPixels.add(`(${row},${col},${currentColor})`);
            }
        }
    }

    function rgbToHex(rgbStr)
    {
        const rgb = rgbStr.match(/^rgb\((\d+),\s*(\d+),\s*(\d+)\)$/);

        if (!rgb)
            return rgbStr;
        
        function toHex(x)
        {
            const hex = parseInt(x).toString(16);
            return hex.length === 1 ? '0' + hex : hex;
        }
        
        // Return in #rrggbb format
        return '#' + toHex(rgb[1]) + toHex(rgb[2]) + toHex(rgb[3]);
    }
    
    // Color picker event
    colorPicker.addEventListener('input', function(e)
    {
        currentColor = e.target.value;

        if (currentColor === '#000000')
        {
            // If black was selected, make it white (since black can't be displayed)
            currentColor = '#ffffff'
        }

        isErasing = false;
        eraserBtn.style.backgroundColor = '#e9e9e9';
    });
    
    // Eraser button
    eraserBtn.addEventListener('click', function()
    {
        isErasing = !isErasing;
        if (isErasing)
        {
            eraserBtn.style.backgroundColor = '#f7c331';
        }
        else
        {
            eraserBtn.style.backgroundColor = '#e9e9e9';
        }
    });
    
    // Clear button
    clearBtn.addEventListener('click', function()
    {
        const pixels = document.querySelectorAll('.pixel');
        pixels.forEach(pixel =>
        {
            pixel.style.backgroundColor = '#000000';
        });
        drawnPixels.clear();
    });
    

    
    sendBtn.addEventListener('click', async (e) => {
        e.preventDefault();
        sendBtn.disabled = true;
        sendBtn.style.cursor = "not-allowed";
        setTimeout(() => {
            sendBtn.disabled = false;           
            sendBtn.style.cursor = "pointer";
        }, 5000);
        let f_list = Array.from(drawnPixels);
        if (f_list.length > 15) {
            for (let i = 0; i < f_list.length; i += 15) {
                let chunk = f_list.slice(i, i + 15).join(',');
                await fetch(`${API_URL}/dashboard/post`, {
                        method: 'POST',
                        headers: { "Content-Type": "application/json" },
                        body: JSON.stringify({
                            "command": "custom",
                            "param": i === 0 ? "start" : "no",
                            "data": chunk
                        })
                });
                await new Promise(resolve => setTimeout(resolve, 1000));
            }
            } else {
            setTimeout(() => {
                sendBtn.disabled = false;
                sendBtn.style.cursor = "pointer";
            }, 3000);
            await fetch(`${API_URL}/dashboard/post`, {
                method: 'POST',
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({
                    "command": "custom",
                    "param": "start",
                    "data": f_list
                })
            });
            await new Promise(resolve => setTimeout(resolve, 2000));
        }
    });
    
    // Initialize board
    createPixelBoard();
});