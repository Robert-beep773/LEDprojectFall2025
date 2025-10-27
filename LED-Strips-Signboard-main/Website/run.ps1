
$http = [System.Net.HttpListener]::new();

$server_port = 8080
$signPort = 0
$baudRate = 0
if(Get-NetTCPConnection -LocalPort $server_port -ErrorAction SilentlyContinue)
{
    do
    {
    $server_port = Read-Host "Enter Server Port"
    if ([string]::IsNullOrWhiteSpace($server_port) -or $server_port -match '\D')
    {
        Write-Host "Invalid input"
    }
    elseif ([int]::Parse($server_port) -ge 1 -and [int]::Parse($server_port) -le 65535)
    {
        
        if(Get-NetTCPConnection -LocalPort $server_port -ErrorAction SilentlyContinue){
            Write-Host "Port is already in use, enter a different one"
        }
        else
        {
            break
        }
    }
    else
    {
        Write-Host "Port number out of range"
    }
}
while($true)
}



do
{
$signPort = Read-Host "Enter Comm PORT"
if ([string]::IsNullOrWhiteSpace($signPort) -or $signPort -match '\D')
{
    Write-Host "Enter a valid COM port"
}
else
{
    break
}
}while($true)

# Fixed baud rate for stability
$baudRate = 9600
Write-Host "Using fixed baud rate: $baudRate"

$url = "http://127.0.0.1:$server_port/";
$http.Prefixes.Add($url);
$http.Start();
$f_port = "COM$signPort" # add the COM to initialize the entered port as serial com


$out_port_a = new-Object System.IO.Ports.SerialPort $f_port,$baudRate,None,8,one

$out_port_a.WriteBufferSize = 65536   # Set the buffer size to high value that is 4096 bytes
$out_port_a.WriteTimeout = 1200 # set max timeout of the serial write operation in order to prevent stalling of main script
$out_port_a.open()

# Log ready message to terminal
if ($http.IsListening) 
{
    Write-Host "Server running on: $url"
    write-host "Connected to the Scoreboard!"
}

# Function to add CORS headers to response
function Add-CorsHeaders($response) 
{
    $response.AppendHeader("Access-Control-Allow-Origin", "*")
    $response.AppendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
    $response.AppendHeader("Access-Control-Allow-Headers", "Content-Type")
}

# Function to convert JSON data to ASCII protocol format
function ConvertTo-AsciiProtocol($jsonData)
{
    $startChar = [char]12  # ASCII 12
    $endChar = [char]15    # ASCII 15
    
    switch ($jsonData.command)
    {
        "static" {
            if ($jsonData.isBig -eq "yes") {
                return "$startChar" + "1002" + $jsonData.data + "$endChar"
            } else {
                return "$startChar" + "1001" + $jsonData.data + "$endChar"
            }
        }
        "scrolC" {
            if ($jsonData.isBig -eq "yes") {
                return "$startChar" + "1004" + $jsonData.data + "$endChar"
            } else {
                return "$startChar" + "1003" + $jsonData.data + "$endChar"
            }
        }
        "scrolS" {
            if ($jsonData.isBig -eq "yes") {
                return "$startChar" + "1006" + $jsonData.data + "$endChar"
            } else {
                return "$startChar" + "1005" + $jsonData.data + "$endChar"
            }
        }
        "fadeIn" {
            if ($jsonData.isBig -eq "yes") {
                return "$startChar" + "1008" + $jsonData.data + "$endChar"
            } else {
                return "$startChar" + "1007" + $jsonData.data + "$endChar"
            }
        }
        "breath" {
            if ($jsonData.isBig -eq "yes") {
                return "$startChar" + "1010" + $jsonData.data + "$endChar"
            } else {
                return "$startChar" + "1009" + $jsonData.data + "$endChar"
            }
        }
        "sTimer" {
            return "$startChar" + "2001" + $jsonData.data + "$endChar"
        }
        "pTimer" {
            return "$startChar" + "2002" + "$endChar"
        }
        "rTimer" {
            return "$startChar" + "2004" + "$endChar"
        }
        "resume" {
            return "$startChar" + "2003" + "$endChar"
        }
        "tod" {
            return "$startChar" + "2006" + "$endChar"
        }
        "settns" {
            $settings = $jsonData.brightness + "," + $jsonData.tcolor + "," + $jsonData.bcolor + "," + $jsonData.fcolor
            return "$startChar" + "3005" + $settings + "$endChar"
        }
        "custom" {
            if ($jsonData.param -eq "start") {
                return "$startChar" + "4002" + "$endChar"  # Clear all pixels first
            }
            # Parse custom pixel data
            $pixelData = $jsonData.data -replace '[()]', '' -replace '#', ''
            return "$startChar" + "4001" + $pixelData + "$endChar"
        }
        default {
            return "$startChar" + "5001" + "$endChar"  # System status request
        }
    }
}

# Main loop
while ($http.IsListening) 
{
    $context = $http.GetContext()
    # Handle preflight OPTIONS requests
    if ($context.Request.HttpMethod -eq 'OPTIONS') {
        Add-CorsHeaders -response $context.Response
        $context.Response.StatusCode = 200
        $context.Response.OutputStream.Close()
        continue
    }
    
# Handle POST requests to /dashboard/post
    if ($context.Request.HttpMethod -eq 'POST' -and $context.Request.RawUrl -eq '/dashboard/post') {
        $FormContent = [System.IO.StreamReader]::new($context.Request.InputStream).ReadToEnd()
        $dataToSend = $FormContent | ConvertFrom-Json
        
        # Convert JSON to new ASCII protocol format
        $asciiCommand = ConvertTo-AsciiProtocol -jsonData $dataToSend
        
        try
        {                                                                                                                      
            $out_port_a.WriteLine($asciiCommand)
            Write-Host "Sent: $asciiCommand"
            Start-Sleep -Milliseconds 200
        }
        catch
        {
            Write-Host "The port timed out, restart the server or wait for a while and than send something else"
            $out_port_a.DiscardOutBuffer() # Discard the Output Buffer so as to remove backlog
            Start-Sleep -Milliseconds 300
        }
        
        Add-CorsHeaders -response $context.Response
        $context.Response.StatusCode = 200
        $context.Response.OutputStream.Close()
        continue
    }
    
    # Handle GET requests
    if ($context.Request.HttpMethod -eq 'GET' -and $context.Request.RawUrl -eq '/kill')
    {
        Add-CorsHeaders -response $context.Response
        $context.Response.StatusCode = 200
        $context.Response.OutputStream.Close()
        $http.Stop()
        break
    }
}







