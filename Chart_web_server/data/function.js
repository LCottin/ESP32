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
    yAxis : 
    {
        title : { text  : 'Humidity (%)' },
        min   : 0,
        max   : 100
    },
    credits  : { enabled  : false }
});

setInterval(function () {
    // Get the data and update the chart each time values are measured
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () 
    {
        if ((this.readyState == 4) && (this.status == 200)) 
        {
            var TChart = [];
            var HChart = [];
            var lines  = this.responseText.split("\n");

            for (var i = 0; i < lines.length; i++) 
            {
                var line = lines[i].split(" ");
                var time = parseInt(line[0]) * 1000;

                TChart.push([time, parseFloat(line[1])]);
                HChart.push([time, parseFloat(line[2])]);
            }
            chartT.series[0].setData(TChart, true);
            chartH.series[0].setData(HChart, true);
        };
    };
    xhttp.open("GET", "/data", true);
    xhttp.send();
}, 2000);
