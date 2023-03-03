function getTimeFromDate(timestamp) 
{
    const date  = new Date(timestamp * 1000);
    var hours   = date.getHours();
    var minutes = date.getMinutes();
    var seconds = date.getSeconds();

    hours   = ("0" + hours).slice(-2);
    minutes = ("0" + minutes).slice(-2);
    seconds = ("0" + seconds).slice(-2);

    return hours + " : " + minutes + " : " + seconds
}

setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () 
    {
        if ((this.readyState == 4) && (this.status == 200)) 
        {
            // Get data from the server
            var data     = this.responseText.split('\n');
            var room0   = data[0].split(';');
            var room1   = data[1].split(';');
            var room2   = data[2].split(';');

            // Update the data
            document.getElementById("room0_id").innerHTML             = room0[0];
            // document.getElementById("room0_time").innerHTML           = getTimeFromDate(parseInt(room0[1]));
            document.getElementById("room0_temperature").innerHTML    = parseFloat(room0[2]);
            document.getElementById("room0_humidity").innerHTML       = parseFloat(room0[3]);
            document.getElementById("room0_pressure").innerHTML       = parseFloat(room0[4]);
            document.getElementById("room0_altitude").innerHTML       = parseFloat(room0[5]);
            // document.getElementById("room0_gas_resistance").innerHTML = parseFloat(room0[6]);

            document.getElementById("room1_id").innerHTML             = room1[0];
            // document.getElementById("room1_time").innerHTML           = getTimeFromDate(parseInt(room1[1]));
            document.getElementById("room1_temperature").innerHTML    = parseFloat(room1[2]);
            document.getElementById("room1_humidity").innerHTML       = parseFloat(room1[3]);
            document.getElementById("room1_pressure").innerHTML       = parseFloat(room1[4]);
            document.getElementById("room1_altitude").innerHTML       = parseFloat(room1[5]);

            document.getElementById("room2_id").innerHTML             = room2[0];
            // document.getElementById("room2_time").innerHTML           = getTimeFromDate(parseInt(room2[1]));
            document.getElementById("room2_temperature").innerHTML    = parseFloat(room2[2]);
            document.getElementById("room2_humidity").innerHTML       = parseFloat(room2[3]);
            document.getElementById("room2_pressure").innerHTML       = parseFloat(room2[4]);
            document.getElementById("room2_altitude").innerHTML       = parseFloat(room2[5]);
        };
    };
    xhttp.open("GET", "/last_data", true);
    xhttp.send();
}, 1000);
