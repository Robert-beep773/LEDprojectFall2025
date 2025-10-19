document.addEventListener("DOMContentLoaded", () =>
{
    const input = document.getElementById("message");
    const input2 = document.getElementById("message2");
    const charCount = document.getElementById("char-count");
    const charCount2 = document.getElementById("char-count2");
    let maxChars = 5;

    // For top and bottom display
    let topBottomElements = document.querySelectorAll('.with-top-bottom');
    let scrollElement = document.querySelector('.with-scroll');

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

        scrollElement.classList.remove('show');

        if (animation === "static")
        {
            if (isBig === "yes")
                maxChars = 5;
            else if (isBig === "no")
                maxChars = 20;

            prevText = input.value;
        }
        else if (animation === "scroll")
        {
            scrollElement.classList.add('show');
            const animationChild = document.querySelector('input[name="scrollType"]:checked').value;
            
            console.log(animationChild);

            if (animationChild === "scrolS")
            {
                if (isBig === "yes")
                    maxChars = 5;
                else if (isBig === "no")
                    maxChars = 20;
    
                prevText = input.value;
            }
            else if (animationChild === "scrolC")
            {
                maxChars = 100;

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

        charCount.textContent = `${input.value.length}/${maxChars}`;
        charCount2.textContent = `${input2.value.length}/${maxChars}`;
    }
});
