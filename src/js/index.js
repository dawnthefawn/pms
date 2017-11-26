var Clay = require('pebble-clay');
var clayConfig = require('./config.json');
var clay = new Clay(clayConfig);

var server_base_url = Clay.getItemsByMessageKey('SERVER_URL');
var sonarr_api_key = Clay.getItemsByMessageKey('SONARR_API');
var sonarr_port = Clay.getItemsByMessageKey('SONARR_PORT');
var pms_service;
var pms_request_type;


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
  if (dict.PMS_SERVICE_SONARR) {
    pms_service = 'SONARR';
    pms_request_type = 'SEARCH';
  }
  console.log('got sonarr search: ' + JSON.stringify(dict));
});

Pebble.addEventListener('appmessage', function(message) {
  var dict = message.payload;
  if (dict.REQUESTTIMELINE) {
    requestTimeline(dict.SERVERURL, dict.SERVERSECRET);
  }
  console.log('got request from pebble app: ' + JSON.stringify(dict));
});

