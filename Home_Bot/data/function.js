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

            //FIXME: get last value instead of first (otherwise, NaN is displayed)
            var label0  = room0[0].split(',');
            var label1  = room1[0].split(',');
            var label2  = room2[0].split(',');

            // Update the data
            document.getElementById("room0_id").innerHTML             = label0[0];
            document.getElementById("room0_time").innerHTML           = getTimeFromDate(parseInt(label0[1]));
            document.getElementById("room0_temperature").innerHTML    = parseFloat(label0[2]);
            document.getElementById("room0_humidity").innerHTML       = parseFloat(label0[3]);
            document.getElementById("room0_pressure").innerHTML       = parseFloat(label0[4]);
            document.getElementById("room0_altitude").innerHTML       = parseFloat(label0[5]);
            document.getElementById("room0_gas_resistance").innerHTML = parseFloat(label0[6]);

            document.getElementById("room1_id").innerHTML             = label1[0];
            document.getElementById("room1_time").innerHTML           = getTimeFromDate(parseInt(label1[1]));
            document.getElementById("room1_temperature").innerHTML    = parseFloat(label1[2]);
            document.getElementById("room1_humidity").innerHTML       = parseFloat(label1[3]);
            document.getElementById("room1_pressure").innerHTML       = parseFloat(label1[4]);
            document.getElementById("room1_altitude").innerHTML       = parseFloat(label1[5]);

            document.getElementById("room2_id").innerHTML             = label2[0];
            document.getElementById("room2_time").innerHTML           = getTimeFromDate(parseInt(label2[1]));
            document.getElementById("room2_temperature").innerHTML    = parseFloat(label2[2]);
            document.getElementById("room2_humidity").innerHTML       = parseFloat(label2[3]);
            document.getElementById("room2_pressure").innerHTML       = parseFloat(label2[4]);
            document.getElementById("room2_altitude").innerHTML       = parseFloat(label2[5]);
        };
    };
    xhttp.open("GET", "/all_data", true);
    xhttp.send();
}, 1000);
