/* Chart for temperature */
var chartT = new Highcharts.Chart({
    chart  : { renderTo  : 'chart-temperature' },
    title  : { text      : 'Temperature' },
    series : 
    [{
        showInLegend : false,
        data         : []
    }],
    plotOptions : 
    {
        line : 
        {
            animation  : false,
            dataLabels : { enabled : true }
        },
    },
    xAxis : 
    {
        type                 : 'datetime',
        dateTimeLabelFormats : { second : '%H:%M:%S' }
    },
    yAxis : 
    {
        title : { text : 'Temperature (Celsius)' },
        min   : -10,
        max   : 40
    },
    credits : { enabled : false }
});

/* Chart for humidity */
var chartH = new Highcharts.Chart({
    chart  : { renderTo : 'chart-humidity' },
    title  : { text     : 'Humidity' },
    series : 
    [{
        showInLegend : false,
        data : []
    }],
    plotOptions : 
    {
        line : 
        {
            animation  : false,
            dataLabels : { enabled : true }
        }
    },
    xAxis : 
    {
        type                 : 'datetime',
        dateTimeLabelFormats : { second : '%H:%M:%S' }
    },
    yAxis  : 
    {
        title : { text  : 'Humidity (%)' },
        min   : 0,
        max   : 100
    },
    credits  : { enabled  : false }
});

setInterval(function () 
{
    // Get the data and update the chart each time values are measured
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () 
    {
        if ((this.readyState == 4) && (this.status == 200)) 
        {
            var x = (new Date()).getTime();
            var y = parseFloat(this.responseText);
            if (chartT.series[0].data.length > 40) 
            {
                chartT.series[0].addPoint([x, y], true, true, true);
            } 
            else 
            {
                chartT.series[0].addPoint([x, y], true, false, true);
            }
        }
    };
    xhttp.open("GET", "/temperature", true);
    xhttp.send();

    xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function ()
    {
        if ((this.readyState == 4) && (this.status == 200)) 
        {
            var x = (new Date()).getTime();
            var y = parseFloat(this.responseText);
            if (chartH.series[0].data.length > 40) 
            {
                chartH.series[0].addPoint([x, y], true, true, true);
            } 
            else 
            {
                chartH.series[0].addPoint([x, y], true, false, true);
            }
        }
    }
    xhttp.open("GET", "/humidity", true);
    xhttp.send();
}, 3000);
