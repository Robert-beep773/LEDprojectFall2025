
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

}while($true)

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







