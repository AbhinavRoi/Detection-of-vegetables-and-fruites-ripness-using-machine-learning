<?php

# Check the incoming data packet:
if(isset($_GET["F1"]) && isset($_GET["F2"]) && isset($_GET["F3"]) && isset($_GET["F4"]) && isset($_GET["F5"]) && isset($_GET["F6"]) && isset($_GET["F7"]) && isset($_GET["F8"]) && isset($_GET["nir_1"]) && isset($_GET["nir_2"]) && isset($_GET["class"])){
	# Create the data array.
	$data = array($_GET["F1"], $_GET["F2"], $_GET["F3"], $_GET["F4"], $_GET["F5"], $_GET["F6"], $_GET["F7"], $_GET["F8"], $_GET["nir_1"], $_GET["nir_2"], $_GET["class"], date("m/d"));
    # Insert the recently generated data array into the CSV file as a new row.	
	$file = fopen("spectral_color_database.csv", "a");
	fputcsv($file, $data);
	fclose($file);
	// Print result:
	echo "Data Inserted Successfully!";
}else{
	echo "Waiting for data from the AS7341 sensor to insert...";
}

?>