var Clay = require('pebble-clay');
var clayConfig = require('./config.json');
//var customClay = require('./custom-clay.js');
//var clay = new Clay(clayConfig, customClay);
var clay = new Clay(clayConfig);
var messageKeys = require('message_keys');

var server_base_url;// = Clay.getItemByMessageKey('SERVER_URL');
var sonarr_api_key;// = Clay.getItemByMessageKey('SONARR_API');
var sonarr_port;// = Clay.getItemByMessageKey('SONARR_PORT');
var sonarr_search_string = '/api/series/lookup?term=';
var sonarr_postfix;// = '&apikey=' + sonarr_api_key;
var pms_service;
var pms_request_type;
var pms_request_url;
var pms_request;
var request_type_string;
var method;
var pms_items;
var pms_tvdbids = {};
var pms_choice;
//function InitializeDefaults() {
//  server_base_url = clay.getItemByMessageKey('SERVER_URL');
//  sonarr_api_key = clay.getItemByMessageKey('SONARR_API');
//  sonarr_port = clay.getItemByMessageKey('SONARR_PORT');
//  sonarr_postfix = '&apikey=' + sonarr_api_key;
//}



function BuildURL() {
  switch (pms_request_type) {
    case 'SEARCH':
      request_type_string = sonarr_search_string;
      method = 'GET';
      break;
    case 'ADD':
      break;
    default:
      return false;
      break;
  }
  switch (pms_service) {
    case 'SONARR':
      pms_request_url = server_base_url + sonarr_port + request_type_string + pms_request + sonarr_postfix;
      break;
    case 'RADARR':
      break;
    default:
      return false;
      break;
  }
  console.log(pms_request_url);
  return true;
}

function ProcessServerResponse(json) {
  var dict = {};
  for (var x = 0; x <= 8; x++) {

    var key = messageKeys.PMS_RESPONSE + x;
    if (x <= json.length) {
      var object = json[x]
      dict[key] = object.title;
      console.log(object.title);
      Pebble.sendAppMessage(dict);
      pms_tvdbids[x] = object.tvdbId;
    }
    if (x > json.length) {  

      console.log('sent last item');
      Pebble.sendAppMessage({'PMS_RESPONSE_SENT':1});
      pms_items = x
      return; 
    }
  }
}

function PmsAddShow(choice) {
  console.log('pms_choice = ' + pms_choice);
  
  console.log(pms_tvdbids[pms_choice]);
}

function SendServerRequest() {
  if (BuildURL() == true) {
    var request = new XMLHttpRequest();
    console.log('Opening request, method: ' + method + 'URL: ' + pms_request_url);
    request.open(method, pms_request_url, true);
    request.onload = function() {
      try {
        
        var json = JSON.parse(this.responseText);
	console.log(json);
        ProcessServerResponse(json);
      } catch(err) {
        console.log('Unable to parse JSON response: ' + err);
      };
    
    
    } 
    request.onerror = function() {
      try {
        var json = JSON.parse(this.responseText);
        console.log(json);
      } catch(err) {
        console.log('Unable to parse JSON Error Message: %s', err);
        }
    }
    console.log('sending request');
    request.send();
  }
}


Pebble.addEventListener('ready', function(e) {
     console.log('PebbleKit JS ready!');
     Pebble.sendAppMessage({'JSReady': 1});
     //InitializeDefaults();
   } );  // Listen for when an AppMessage is received Pebble.addEventListener('appmessage',   function(e) {     console.log('AppMessage received!');   } );

Pebble.addEventListener('appmessage', function(message) {
  var dict = message.payload;
  if (dict.PMS_CHOICE) {
    console.log('dict.PMS_CHOICE= ' + dict.PMS_CHOICE);
    pms_choice=dict.PMS_CHOICE;
    PmsAddShow(pms_choice);
    return;
  }
  if (dict.PMS_REQUEST) {
    console.log('dict.PMS_REQUEST= ' + encodeURI(dict.PMS_REQUEST));
    pms_request = escape(dict.PMS_REQUEST);
  }
  console.log('got request string: ' + pms_request);
  SendServerRequest();
  dict.PMS_REQUEST = undefined;
});

  
      
  


Pebble.addEventListener('appmessage', function(message) {
  var dict = message.payload;
  if (dict.PMS_SERVICE_SONARR) {
    pms_service = 'SONARR';
    pms_request_type = 'SEARCH';

    console.log('got service request: ' + JSON.stringify(dict));
  }
  if (dict.SERVER_URL) {
    server_base_url = dict.SERVER_URL;
    console.log('set server url as: ' + server_base_url);
  }
  if (dict.SONARR_API) {
    sonarr_api_key = dict.SONARR_API;
    console.log('set sonarr api as: ' + sonarr_api_key);
  }
  if (dict.SONARR_PORT) {
    sonarr_port = dict.SONARR_PORT;   
    sonarr_postfix = '&apikey=' + sonarr_api_key;
    console.log('set sonarr port as: ' + sonarr_port);
  }
});

