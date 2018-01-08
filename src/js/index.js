var Clay = require('pebble-clay');
var clayConfig = require('./config.json');
//var customClay = require('./custom-clay.js');
//var clay = new Clay(clayConfig, customClay);
var clay = new Clay(clayConfig);
var messageKeys = require('message_keys');

var server_base_url;// = Clay.getItemByMessageKey('SERVER_URL');
var sonarr_api_key;// = Clay.getItemByMessageKey('SONARR_API');
var radarr_api_key;
var api_key;
var radarr_port;
var sonarr_port;// = Clay.getItemByMessageKey('SONARR_PORT');
var port;
var sonarr_search_string = '/api/series/lookup?term=';
var radarr_search_string = '/api/movies/lookup?term=';
var radarr_add_string = '/api/movie';
var sonarr_add_string = '/api/series';
var search_postfix = '&apikey=';
var add_postfix = '?apikey=';
var postfix;
var pms_service;
var pms_request_type;
var pms_request_url;
var pms_request;
var request_type_string;
var service_type_string;
var api_key_postfix;
var method;
var pms_items;
var pms_tvdbids = {};
var pms_choice;
var json;

function AddMedia(request, choice) {
  console.log('AddMedia(): request = ' + request);
  var serverrequest = new XMLHttpRequest();
    pms_request_type = 'ADD';
    if (BuildURL() == true) {

      console.log(pms_request_url);
      console.log(JSON.parse(request));
      serverrequest.open(method, pms_request_url, true);
      serverrequest.onload = function() {
        try {
          
          var reply = JSON.parse(this.responseText);
  	  console.log(reply);
	  if (reply.title == json[choice].title) {
	    Pebble.sendAppMessage({'PMS_SUCCESS': 1});
          } else {
            Pebble.sendAppMessage({'PMS_ERROR': 'Item Added is not Item Selected'});
          } 
        } catch(err) {
          console.log('Unable to parse JSON response: ' + err);
          Pebble.sendAppMessage({'PMS_ERROR': 'Failed to parse server response'});
          return false;
        };
      
      
      } 
      serverrequest.onerror = function() {
        try {
          var reply = JSON.parse(this.responseText);
          console.log(reply);
          Pebble.sendAppMessage({'PMS_ERROR': reply});
          return false;
        } catch(err) {
          console.log('Unable to parse JSON Error Message: ', err);
          Pebble.sendAppMessage({'PMS_ERROR': 'Failed to parse error message'});
          return false;
          }
      }
      console.log('sending request');
      serverrequest.send(request);
      }
      else {
	console.log('BuildURL failed');
        return false;
      }
      return true;
  }


function BuildURL() {
  console.log ('BuildURL(): pms_service = ' + pms_service + '; pms_request_type = ' + pms_request_type + ';');
  if (pms_service == 'SONARR') {
    port = ':' + sonarr_port;
    api_key = sonarr_api_key;
    if (pms_request_type == 'ADD') {
      request_type_string = sonarr_add_string;
      postfix = add_postfix;
      method = 'POST';
      pms_request = '';
    }
    else if (pms_request_type == 'SEARCH') {
      request_type_string = sonarr_search_string + pms_request;
      postfix = search_postfix;
      method = 'GET';
    }
    else {
      return false;

    }
  }
  else if (pms_service == 'RADARR') {
    port = ':' + radarr_port;
    api_key = radarr_api_key;

    if (pms_request_type == 'ADD') {
      request_type_string = radarr_add_string;
      postfix = add_postfix;
      method = 'POST';
      pms_request = '';
    }
    else if (pms_request_type == 'SEARCH') {
      request_type_string = radarr_search_string + pms_request;
      postfix = search_postfix;
      method = 'GET';
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
  
  pms_request_url = server_base_url + port + request_type_string + postfix + api_key;
  return true;
}

function ProcessServerResponse(dict, x) {

    var key = messageKeys.PMS_RESPONSE + x;
    if (x < json.length) 
	{
      var object = json[x];
      dict[key] = object.title;
	  console.log(object.title);
	  Pebble.sendAppMessage({"PMS_RESPONSE_INDEX":x + 1});
      Pebble.sendAppMessage(dict, function() {
        
        if (x < 7) {
          x = x + 1;
          ProcessServerResponse(dict, x);

        }
		else
		{
			pms_items = x;
	   		console.log('sent last item');
			Pebble.sendAppMessage({'PMS_RESPONSE_SENT':1, 'PMS_RESPONSE_ITEMS':pms_items + 1});
			return;
		}
	});
    }
	else 
	{
		pms_items = x;
   		console.log('sent last item');
		Pebble.sendAppMessage({'PMS_RESPONSE_SENT':1, 'PMS_RESPONSE_ITEMS':pms_items + 1});
		return;
	}
}

function PmsBuildRequest(choice) {
  console.log('Beginning PmsBuildRequest();');
  console.log('pms_choice = ' + choice);
  choice = choice - 1;
//  console.log(pms_tvdbids[pms_choice]);
  var searchresult = json[choice];
  console.log(searchresult);
  console.log('pms_service is ' + pms_service);
    if (pms_service == 'SONARR') { 
      var request = JSON.stringify({"tvdbId": searchresult.tvdbId, "title": searchresult.title, "qualityProfileId": 1, "titleSlug": searchresult.titleSlug, "images": searchresult.images, "seasons": searchresult.seasons, "rootFolderPath": "/ldm/TV/"}, null, "\t");
    }
    else if (pms_service == 'RADARR') { 
      var request = JSON.stringify({"tmdbId": searchresult.tmdbId, "title": searchresult.title, "qualityProfileId": 1, "titleSlug": searchresult.titleSlug, "images": searchresult.images, "rootFolderPath": "/ldm/Movies/"}, null, "\t");
    }
    else {
      console.log('PmsBuildRequest() Failed');
      return false;
    }
  

  console.log(request);
  if (AddMedia(request, choice) == true) {
    return true;
  }
  else { 
    console.log('PmsBuildRequest(): AddMedia() == False');
    return false;
  }
}

function SendServerRequest() {
  if (BuildURL() == true) {
    var request = new XMLHttpRequest();
    console.log('Opening request, method: ' + method + 'URL: ' + pms_request_url);
    request.open(method, pms_request_url, true);
    request.onload = function() {
      try {
        json = JSON.parse(this.responseText);
        pms_request_type = 'ADD';
        ProcessServerResponse({}, 0);
      } catch(err) {
		console.log(this.responseText);
        console.log('Unable to parse JSON response: ' + err);
      };
    
    
    } 
    request.onerror = function() {
      try {
        json = JSON.parse(this.responseText);
      } catch(err) {
        console.log(this.responseText);
        console.log('Unable to parse JSON Error Message: ', err);
        }
    }
    console.log('sending request');
    request.send();
  }
  else {
    console.log('SendServerRequest failed: BuildURL == False');
  }
}


Pebble.addEventListener('ready', function(e) {
     console.log('PebbleKit JS ready!');
     Pebble.sendAppMessage({'JSReady': 1});
   } );  

Pebble.addEventListener('appmessage', function(message) {
  var dict = message.payload;
  if (dict.PMS_CHOICE) {
    console.log('dict.PMS_CHOICE= ' + dict.PMS_CHOICE);
    pms_choice=dict.PMS_CHOICE;
    if (PmsBuildRequest(pms_choice) == true) { 
      console.log('PmsBuildRequest == true');
    }
    else {
      console.log('PmsBuildRequest == false');
    }
    pms_request = '';
    return;
  }
  if (dict.PMS_REQUEST) {
    console.log('dict.PMS_REQUEST=' + encodeURI(dict.PMS_REQUEST));
    pms_request = encodeURI(dict.PMS_REQUEST);
    console.log('got request string: ' + pms_request);
    SendServerRequest();
    dict.PMS_REQUEST = '';
  }
});

  
      
  


Pebble.addEventListener('appmessage', function(message) {
  var dict = message.payload;
  if (dict.PMS_SERVICE_SONARR) {
    pms_service = 'SONARR';
    pms_request_type = 'SEARCH';
    console.log('got service request: ' + dict);
    dict.PMS_SERVICE_SONARR = 0;
    return;
  }
  else if (dict.PMS_SERVICE_RADARR) {
    pms_service = 'RADARR';
    pms_request_type = 'SEARCH';
    dict.PMS_SERVICE_RADARR = 0;
    return;
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
    console.log('set sonarr port as: ' + sonarr_port);
  }
  if (dict.RADARR_API) {
    radarr_api_key = dict.RADARR_API;
    console.log('set radarr api as: ' + radarr_api_key);
  }
  if (dict.RADARR_PORT) {
    radarr_port = dict.RADARR_PORT;
    console.log('set radarr port as: ' + radarr_port);
  }
});

