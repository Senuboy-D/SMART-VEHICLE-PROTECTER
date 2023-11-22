<!DOCTYPE html>
<html><body>
<?php

$servername = "localhost";

$dbname = "tracking";
$username = "root";
$password = "";


$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "SELECT * FROM historydata ORDER BY id DESC";

echo '<table cellspacing="5" cellpadding="5">
      <tr> 
        <td>Temperature</td> 
        <td>Latitude</td> 
        <td>Longitude</td> 
        <td>accident_Status</td> 
        <td>Speed</td> 
        <td>Date Time</td> 
      </tr>';
 
if ($result = $conn->query($sql)) {
    while ($row = $result->fetch_assoc()) {
        $row_id = $row["Temperature"];
        $row_id1 = $row["Latitude"];
        $row_id2 = $row["Longitude"];
        $row_id3 = $row["accident"];
        $row_id4 = $row["Speed"];
        $row_reading_time = $row["reading_time"];
     
        echo '<tr> 
                <td>' . $row_id . '</td> 
                <td>' . $row_id1 . '</td> 
                <td>' . $row_id2 . '</td> 
                <td>' . $row_id3 . '</td> 
                <td>' . $row_id4 . '</td> 
                <td>' . $row_reading_time . '</td> 
              </tr>';
    }
    $result->free();
}

$conn->close();
?> 
</table>
</body>
</html>