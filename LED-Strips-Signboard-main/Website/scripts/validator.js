document.addEventListener("DOMContentLoaded", () =>
{
    const input = document.getElementById("message");
    const input2 = document.getElementById("message2");
    const charCount = document.getElementById("char-count");
    const charCount2 = document.getElementById("char-count2");
    let maxChars = 5;

    // For top and bottom display
    let topBottomElements = document.querySelectorAll('.with-top-bottom');
    let scrollElements = document.querySelectorAll('.with-scroll');

    // This is for helping users keep the test they typed for maxChars = 20
    let prevText = "";

    document.querySelectorAll('input[name="isBig"]').forEach((radio) =>
    {
        radio.addEventListener("change", function ()
        {
            updateMaxCount();
        });
    });

    
    document.querySelectorAll('input[name="animation"]').forEach((radio) =>
    {
        radio.addEventListener("change", function ()
        {
            updateMaxCount();
        });
    });

    document.querySelectorAll('input[name="scrollType"]').forEach((radio) =>
    {
        radio.addEventListener("change", function ()
        {
            updateMaxCount();
        });
    });

    input.addEventListener("input", () =>
    {
        updateMaxCount();
    });

    input2.addEventListener("input", () =>
    {
        updateMaxCount();
    });

    function updateMaxCount()
    {
        const isBig = document.querySelector('input[name="isBig"]:checked').value;
        const animation = document.querySelector('input[name="animation"]:checked').value;

        // Hide all scroll options
        scrollElements.forEach(function(element) {
            element.classList.remove('show');
        });

        if (animation === "static")
        {
            if (isBig === "yes")
                maxChars = 4;  // Full screen: 4 characters max
            else if (isBig === "no")
                maxChars = 10;  // Top/Bottom: 10 characters max

            prevText = input.value;
        }
        else if (animation === "scroll")
        {
            // Show all scroll options (scroll type and scroll speed)
            scrollElements.forEach(function(element) {
                element.classList.add('show');
            });
            const animationChild = document.querySelector('input[name="scrollType"]:checked').value;
            
            console.log(animationChild);

            if (animationChild === "scrolS")
            {
                // Scroll then stop: same limits as static
                if (isBig === "yes")
                    maxChars = 4;  // Full screen: 4 characters max
                else if (isBig === "no")
                    maxChars = 10;  // Top/Bottom: 10 characters max
    
                prevText = input.value;
            }
            else if (animationChild === "scrolC")
            {
                // Continuous scroll: allow up to 120 characters (regardless of full/top-bottom)
                maxChars = 120;

                if (prevText.startsWith(input.value))
                {
                    input.value = prevText;
                    prevText = "";
                }
            }
        }

        // Updating the displayed text boxes
        if (isBig === "yes")
        {
            topBottomElements.forEach(function(element)
            {
                element.classList.remove('show');
            });
        }
        else if (isBig === "no")
        {
            topBottomElements.forEach(function(element)
            {
                element.classList.add('show');
            });
        }

        // For scroll continuous, don't trim - just show count (validation happens on send)
        // For other modes, trim excess characters
        const scrollType = animation === "scroll" ? document.querySelector('input[name="scrollType"]:checked')?.value : null;
        
        if (animation === "scroll" && scrollType === "scrolC")
        {
            // Scroll continuous: show count but don't trim (validation in Dashboard.js)
            charCount.style.color = input.value.length > maxChars ? "orange" : "black";
            charCount2.style.color = input2.value.length > maxChars ? "orange" : "black";
        }
        else
        {
            // Static and scroll then stop: trim excess characters
            if (input.value.length > maxChars)
            {
                input.value = input.value.substring(0, maxChars); // Trims excess characters
                charCount.style.color = "red";
            }
            else
            {
                charCount.style.color = "black";
            }

            if (input2.value.length > maxChars)
            {
                input2.value = input2.value.substring(0, maxChars); // Trims excess characters
                charCount2.style.color = "red";
            }
            else
            {
                charCount2.style.color = "black";
            }
        }

        charCount.textContent = `${input.value.length}/${maxChars}`;
        charCount2.textContent = `${input2.value.length}/${maxChars}`;
    }
});
