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
            var data                = this.responseText.split(' ');
            var time                = parseInt(data[0]);
            var Temperature_dht11   = parseFloat(data[1]);
            var Humidity_dht11      = parseFloat(data[2]);
            var Temperature_bme280  = parseFloat(data[3]);
            var Humidity_bme280     = parseFloat(data[4]);
            var Pressure_bme280     = parseFloat(data[5]);

            // Update the data
            document.getElementById("time").innerHTML               = getTimeFromDate(time);
            document.getElementById("temperature_dht11").innerHTML  = Temperature_dht11;
            document.getElementById("humidity_dht11").innerHTML     = Humidity_dht11;
            document.getElementById("temperature_bme280").innerHTML = Temperature_bme280;
            document.getElementById("humidity_bme280").innerHTML    = Humidity_bme280;
            document.getElementById("pressure_bme280").innerHTML    = Pressure_bme280;
        };
    };
    xhttp.open("GET", "/data", true);
    xhttp.send();
}, 1000);
