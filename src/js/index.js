var Clay = require('pebble-clay');
var clayConfig = require('./config.json');
var clay = new Clay(clayConfig);

var sonarr_api_key = 'd8b01e33441bb82e0d9b0083b453';
var sonarr_search_url = 'http://192.168.1.100:8989/api/series/lookup?term=';


Pebble.AddEventListener('ready', function(e) {
     console.log('PebbleKit JS ready!');
     Pebble.sendAppMessage({'JSReady': 1});
   } );  // Listen for when an AppMessage is received Pebble.addEventListener('appmessage',   function(e) {     console.log('AppMessage received!');   } );
Pebble.addEventListener('appmessage', function(message) {
  var dict = message.payload;
  if (dict.REQUESTTIMELINE) {
    requestTimeline(dict.SERVERURL, dict.SERVERSECRET);
  }
  console.log('got request from pebble app: ' + JSON.stringify(dict));
});


Pebble.addEventListener('appmessage', function(message) {
  var dict = message.payload;
  if (dict.REQUESTTIMELINE) {
    requestTimeline(dict.SERVERURL, dict.SERVERSECRET);
  }
  console.log('got request from pebble app: ' + JSON.stringify(dict));
});

