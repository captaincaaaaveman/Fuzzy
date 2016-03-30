var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    console.log(this.responseText);
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};



function getStocks() {
  // Construct URL
  var url = "http://finance.google.com/finance/info?client=ig&q=SXX";
  var url2 = "http://finance.google.com/finance/info?client=ig&q=INDEXFTSE:UKX";

  var d = new Date();

  if (d.getHours() > 16 ) {
    console.log("After 4:30hrs looking up DJI");
    url2 = "http://finance.google.com/finance/info?client=ig&q=INDEXDJX:.DJI";
  }
  
  console.log('LOOKING UP...');
  console.log(url);

  xhrRequest(url, 'GET', 
    function(responseText) {
      console.log('LOOKED UP...');
      console.log(responseText);
      console.log('...');
      
      responseText = responseText.substring(3);
      
      console.log(responseText);
      console.log('...');

      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      console.log('parsed ' + json);
      console.log('parsed ' + json[0].t);

      // Temperature in Kelvin requires adjustment
      var temperature = json[0].t;
      console.log('Ticker is ' + temperature);

      // Conditions
      var conditions = json[0].l;      

      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_SXX': conditions
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
      function(e) {
        console.log('Weather info sent to Pebble successfully!');
      },
      function(e) {
        console.log('Error sending weather info to Pebble!');
        }
      );
    }      
  );
  
  xhrRequest(url2, 'GET', 
  function(responseText) {
      console.log('LOOKED UP...');
      console.log(responseText);
      console.log('...');
      
      responseText = responseText.substring(3);
      
      console.log(responseText);
      console.log('...');

      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      console.log('parsed ' + json);
      console.log('parsed ' + json[0].c);

      // Temperature in Kelvin requires adjustment
      var temperature = json[0].c;
      console.log('Ticker is ' + temperature);

      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_FTSE': temperature
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
      function(e) {
        console.log('Weather info sent to Pebble successfully!');
      },
      function(e) {
        console.log('Error sending weather info to Pebble!');
        }
      );
    }      
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial weather
    getStocks();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getStocks();
  }                     
);
