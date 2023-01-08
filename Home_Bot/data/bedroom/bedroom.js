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

/* Chart for pressure */
var chartP = new Highcharts.Chart({
    chart  : { renderTo : 'chart-pressure' },
    title  : { text     : 'Pressure' },
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
        title : { text  : 'Pressure (hPa)' },
        min   : 950,
        max   : 1050
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
            var PChart = [];

            // Get data from the server
            var data  = this.responseText.split('\n');
            var room = data[1].split(';');

            for (var i = 0; i < room.length; i++) 
            {
                var time = parseInt(room[1]) * 1000;

                // Update the charts
                TChart.push([time, parseFloat(room[2])]);
                HChart.push([time, parseFloat(room[3])]);
                PChart.push([time, parseFloat(room[4])]);
            }
            chartT.series[0].setData(TChart, true);
            chartH.series[0].setData(HChart, true);
            chartP.series[0].setData(PChart, true);
        };
    };
    xhttp.open("GET", "/data", true);
    xhttp.send();
}, 10000);
