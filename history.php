<?php

$servername = "localhost";


$dbname = "tracking";
$username = "root";
$password = "";

$api_key_value = "tPmAT5Ab3j7F9";

$api_key= $Temperature = $Latitude = $Longitude = $accident = $Speed = "";

if ($_SERVER["REQUEST_METHOD"] == "GET") {
    $api_key = test_input($_GET["api_key"]);
    if($api_key == $api_key_value) {
        $Temperature = test_input($_GET["Temperature"]);
		$Latitude = test_input($_GET["Latitude"]);
		$Longitude = test_input($_GET["Longitude"]);
		$accident = test_input($_GET["accident"]);
		$Speed = test_input($_GET["Speed"]);
        
        // Create connection
        $conn = new mysqli($servername, $username, $password, $dbname);
        // Check connection
        if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        } 
        
        $sql = "INSERT INTO historydata (Temperature,Latitude,Longitude,accident,Speed)
        VALUES ('" . $Temperature . "','" . $Latitude . "','" . $Longitude . "','" . $accident . "','" . $Speed . "')";
        
        if ($conn->query($sql) === TRUE) {
            echo "New record created successfully";
        } 
        else {
            echo "Error: " . $sql . "<br>" . $conn->error;
        }
    
        $conn->close();
    }
    else {
        echo "Wrong API Key provided.";
    }

}
else {
    echo "No data posted with HTTP POST.";
}

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}