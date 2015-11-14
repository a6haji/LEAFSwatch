var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};


function locationSuccess(pos) {
  // Construct URL
  var url = "http://live.nhle.com/GameData/RegularSeasonScoreboardv3.jsonp";

  // Send request to nhl.com
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with score info
      var jsonText = responseText.substring(15, responseText.length - 1);
      console.log("This is cut json:" + jsonText);
      var json = JSON.parse(jsonText);
      console.log("This is parsed json:" + json);
      
      // Find the desired game array
      var gamearraynumber = findLeafs(json);
      console.log(gamearraynumber);
      
      // Collect desired data
      var game_time = "";
      if (gamearraynumber == "No Game"){
        game_time = "No Game";
      }
      else {
        //Game-Info
        game_time = json.games[gamearraynumber].ts;
        console.log(game_time);
        var home_team = json.games[gamearraynumber].htn;
        console.log(home_team);
        var away_team = json.games[gamearraynumber].atn;
        console.log(away_team);
        var home_score = json.games[gamearraynumber].hts;
        console.log(home_score);
        var away_score = json.games[gamearraynumber].ats;
        console.log(away_score);
      }

      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE": null,
        "KEY_CONDITIONS": game_time
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }      
  );
}

function locationError(err) {
  console.log("Error requesting location!");
}

function findLeafs(json) {
  for(var i=0; i < json.games.length; i++){
    if ((json.games[i].htn == "Calgary") && ((json.games[i].ts == "TODAY")||(json.games[i].tsc == "progress"))){
      return i;
    }
    if ((json.games[i].atn == "Calgary") && ((json.games[i].ts == "TODAY")||(json.games[i].tsc == "progress"))){
      return i;
    }
  }
      return "No Game";
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getWeather();
  }                     
);
