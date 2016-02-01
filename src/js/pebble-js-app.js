var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

// Get the Day of the Week
  var date = new Date();
  var weekdays = ["SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"];
  var today = weekdays[date.getDay()];


function Scores(pos) {
  // Construct URL
  var url = "http://live.nhle.com/GameData/RegularSeasonScoreboardv3.jsonp";

  // Send request to nhl.com
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with score info
      var jsonText = responseText.substring(15, responseText.length - 1);
      console.log("This is cut json:" + jsonText);
      var json = JSON.parse(jsonText);
      
      // Find the desired game array
      var gamearraynumber = findLeafs(json);
      
      // Collect desired data
      var game_time = "";
      var period = "";
      var home_team = "";
      var away_team = "";
      var home_score = "";
      var away_score = "";

      if (gamearraynumber == "No Game"){
        game_time = "No Game";
      }
      else {
        //Game-Info
        period = json.games[gamearraynumber].ts + " -";
        if (period.substring(0,3) == today){
          period = " ";
        }
        console.log(period);
        game_time = json.games[gamearraynumber].bs;
        console.log(game_time);
        home_team = json.games[gamearraynumber].htn.substring(0,3).toUpperCase() + ": ";
        console.log(home_team);
        away_team = json.games[gamearraynumber].atn.substring(0,3).toUpperCase() + ": ";
        console.log(away_team);
        home_score = json.games[gamearraynumber].hts;
        console.log(home_score);
        away_score = json.games[gamearraynumber].ats;
        console.log(away_score);
      }

      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_PERIOD": period,
        "KEY_GAME_TIME": game_time,
        "KEY_HOME_TEAM": home_team,
        "KEY_HOME_SCORE": home_score,
        "KEY_AWAY_TEAM": away_team,
        "KEY_AWAY_SCORE": away_score
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

function findLeafs(json) {
  for(var i=0; i < json.games.length; i++){
    if ((json.games[i].htn == "Toronto") && 
        ((json.games[i].ts == "TODAY") ||
         (json.games[i].tsc == "progress") ||
         (json.games[i].tsc == "critical") ||
         (today === json.games[i].ts.substring(0,3)))){
      return i;
    }
    if ((json.games[i].atn == "Toronto") && 
        ((json.games[i].ts == "TODAY") ||
         (json.games[i].tsc == "progress") ||
         (json.games[i].tsc == "critical") ||
         (today === json.games[i].ts.substring(0,3)))){
      return i;
    }
  }
      return "No Game";
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the Scores
    Scores();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    Scores();
  }                     
);
