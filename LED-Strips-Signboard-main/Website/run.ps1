
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

<<<<<<< Updated upstream
do
{
    $baudRate = Read-Host "Enter Baud Rate"
    if ([string]::IsNullOrWhiteSpace($baudRate) -or $baudRate -match '\D')
    {
        Write-Host "Enter a valid Baud rate"
    }
else
{
    break
}

=======
# Ask for baud rate with validation
do
{
$baudInput = Read-Host "Enter Baud Rate"
if ([string]::IsNullOrWhiteSpace($baudInput) -or $baudInput -match '\D')
{
    Write-Host "Invalid input. Enter numbers only."
}
else
{
    $parsedBaud = [int]::Parse($baudInput)
    if ($parsedBaud -ge 1200 -and $parsedBaud -le 115200)
    {
        $baudRate = $parsedBaud
        Write-Host "Using baud rate: $baudRate"
        break
    }
    else
    {
        Write-Host "Baud rate must be between 1200 and 115200"
    }
}
>>>>>>> Stashed changes
}while($true)

$url = "http://127.0.0.1:$server_port/";
$http.Prefixes.Add($url);
$http.Start();
$f_port = "COM$signPort" # add the COM to initialize the entered port as serial com

# Connect at 9600 first to send baud rate to Arduino
$initialBaud = 9600
$out_port_a = new-Object System.IO.Ports.SerialPort $f_port,$initialBaud,None,8,one
$out_port_a.WriteBufferSize = 65536
$out_port_a.WriteTimeout = 1200
$out_port_a.open()

# Send baud rate to Arduino (just the number, Arduino will read it)
Write-Host "Sending baud rate $baudRate to Arduino..."
$out_port_a.WriteLine($baudRate)
Start-Sleep -Milliseconds 500  # Give Arduino time to process and switch

# Close and reopen at the new baud rate
$out_port_a.Close()
Start-Sleep -Milliseconds 200
$out_port_a = new-Object System.IO.Ports.SerialPort $f_port,$baudRate,None,8,one
$out_port_a.WriteBufferSize = 65536
$out_port_a.WriteTimeout = 1200
$out_port_a.open()
Write-Host "Reconnected at baud rate: $baudRate"

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

<<<<<<< Updated upstream
=======
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
            # Use different command codes for fast/slow scroll
            # Fast: 1013/1014, Slow: 1003/1004
            $isFast = $jsonData.scrollSpeed -ne "slow"
            if ($jsonData.isBig -eq "yes") {
                $cmdCode = if ($isFast) { "1014" } else { "1004" }
                return "$startChar" + $cmdCode + $jsonData.data + "$endChar"
            } else {
                $cmdCode = if ($isFast) { "1013" } else { "1003" }
                return "$startChar" + $cmdCode + $jsonData.data + "$endChar"
            }
        }
        "scrolS" {
            # Use different command codes for fast/slow scroll
            # Fast: 1015/1016, Slow: 1005/1006
            $isFast = $jsonData.scrollSpeed -ne "slow"
            if ($jsonData.isBig -eq "yes") {
                $cmdCode = if ($isFast) { "1016" } else { "1006" }
                return "$startChar" + $cmdCode + $jsonData.data + "$endChar"
            } else {
                $cmdCode = if ($isFast) { "1015" } else { "1005" }
                return "$startChar" + $cmdCode + $jsonData.data + "$endChar"
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
            # Handle null or empty values - convert to strings to avoid type errors
            $brightness = if ([string]::IsNullOrEmpty($jsonData.brightness)) { "" } else { [string]$jsonData.brightness }
            $tcolor = if ([string]::IsNullOrEmpty($jsonData.tcolor)) { "" } else { [string]$jsonData.tcolor }
            $bcolor = if ([string]::IsNullOrEmpty($jsonData.bcolor)) { "" } else { [string]$jsonData.bcolor }
            $fcolor = if ([string]::IsNullOrEmpty($jsonData.fcolor)) { "" } else { [string]$jsonData.fcolor }
            $settings = $brightness + "," + $tcolor + "," + $bcolor + "," + $fcolor
            return "$startChar" + "3005" + $settings + "$endChar"
        }
        "custom" {
            if ($jsonData.param -eq "start") {
                return "$startChar" + "4002" + "$endChar"  # Clear all pixels first
            }
            if ($jsonData.param -eq "row") {
                # Row-based format: row,col1,color1,col2,color2,...
                return "$startChar" + "4004" + $jsonData.data + "$endChar"
            }
            # Legacy single pixel format (for backwards compatibility)
            $pixelData = $jsonData.data -replace '[()]', '' -replace '#', ''
            return "$startChar" + "4001" + $pixelData + "$endChar"
        }
        default {
            return "$startChar" + "5001" + "$endChar"  # System status request
        }
    }
>>>>>>> Stashed changes
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
        $data = ''
        if($dataToSend.command -eq "pTimer")
        {
            $data = "$" + $dataToSend.command + "$"
        }
        elseif($dataToSend.command -eq "rTimer")
        {
            $data =  "$" + $dataToSend.command  + "$"
        }
        elseif($dataToSend.command -eq "resume")
        {
            $data = "$" + $dataToSend.command + "$"
        }
        elseif($dataToSend.command -eq "tod")
        {
            $data =  "$" + $dataToSend.command + "$"
        }
        elseif($dataToSend.command -eq "settns")
        {
            $data =  "$" + $dataToSend.command + "$" +  "[" + $dataToSend.brightness + ", " +  $dataToSend.tcolor + ", " +  $dataToSend.bcolor + ", " + $dataToSend.fcolor + "]"
        }
        elseif($dataToSend.command -eq "custom")
        {
            $data =  "$" + $dataToSend.command + "$" + $dataToSend.param + "[" + $dataToSend.data + "]"
        }
        else
        {
            $data = "$" + $dataToSend.command + "$" + $dataToSend.isBig + "[" + $dataToSend.data + "]"
        }
     
        try
        {                                                                                                                      
            # $sw = [System.Diagnostics.Stopwatch]::StartNew()
            $out_port_a.WriteLine($data)
            Write-Host $data
            Start-Sleep -Milliseconds 200
            #$sw.Stop()

            # Write-Host "Execution Time: $($sw.ElapsedMilliseconds) ms"
        }
        catch
        {
            Write-Host "The port timed out, restart the server or wait for a while and than send omwthing else"
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







