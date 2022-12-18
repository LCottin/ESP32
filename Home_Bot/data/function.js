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
            var data        = this.responseText.split('\n');
            var living_room = data[0].split(';');
            var bedroom     = data[1].split(';');

            // Update the data
            document.getElementById("room0_id").innerHTML             = living_room[0];
            document.getElementById("room0_time").innerHTML           = getTimeFromDate(parseInt(living_room[1]));
            document.getElementById("room0_temperature").innerHTML    = parseFloat(living_room[2]);
            document.getElementById("room0_humidity").innerHTML       = parseFloat(living_room[3]);
            document.getElementById("room0_pressure").innerHTML       = parseFloat(living_room[4]);
            document.getElementById("room0_altitude").innerHTML       = parseFloat(living_room[5]);
            document.getElementById("room0_gas_resistance").innerHTML = parseFloat(living_room[6]);

            document.getElementById("room1_id").innerHTML             = bedroom[0];
            document.getElementById("room1_time").innerHTML           = getTimeFromDate(parseInt(bedroom[1]));
            document.getElementById("room1_temperature").innerHTML    = parseFloat(bedroom[2]);
            document.getElementById("room1_humidity").innerHTML       = parseFloat(bedroom[3]);
            document.getElementById("room1_pressure").innerHTML       = parseFloat(bedroom[4]);
            document.getElementById("room1_altitude").innerHTML       = parseFloat(bedroom[5]);
        };
    };
    xhttp.open("GET", "/data", true);
    xhttp.send();
}, 1000);
