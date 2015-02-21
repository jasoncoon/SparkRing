var app = angular.module('app', ['colorpicker.module']);
// var app = angular.module('app', []);

app.config(function ($httpProvider) {
  $httpProvider.defaults.transformRequest = function (data) {
    if (data === undefined) {
      return data;
    }
    return $.param(data);
  };
  $httpProvider.defaults.headers.post['Content-Type'] = 'application/x-www-form-urlencoded;';
});

app.controller('MainCtrl', function ($scope, $http, $timeout, patternService) {
  $scope.brightness = "";
  $scope.busy = false;
  // $scope.pattern = "";
  $scope.timezone = 0;
  $scope.power = 1;
  $scope.flipClock = 0;
  $scope.r = 0;
  $scope.g = 0;
  $scope.b = 255;
  $scope.powerText = "On";
  $scope.status = "Please enter your device ID and access token.";
  $scope.disconnected = true;

  $scope.patterns = [];
  $scope.patternIndex = 0;

  $scope.status = 'Loading settings...';

  chrome.storage.sync.get('deviceId',
    function(result) {
      $scope.status = 'Loaded device ID';
      $scope.deviceId = result.deviceId;
    }
  );

  chrome.storage.sync.get('accessToken',
    function(result) {
      $scope.status = 'Loaded access token';
      $scope.accessToken = result.accessToken;
    }
  );

  $scope.save = function () {
    $scope.status = 'Saving settings...';
    chrome.storage.sync.set({'deviceId': $scope.deviceId, 'accessToken': $scope.accessToken},
    function() {
      $scope.status = 'Settings saved';
      // message('Settings saved');
    });
  }

  $scope.connect = function() {
    $scope.busy = true;

    $http.get('https://api.spark.io/v1/devices/' + $scope.deviceId + '/power?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.power = data.result;
        $scope.status = 'Loaded power';
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });

    $http.get('https://api.spark.io/v1/devices/' + $scope.deviceId + '/timezone?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.timezone = data.result;
        $scope.status = 'Loaded time zone';
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });

    $http.get('https://api.spark.io/v1/devices/' + $scope.deviceId + '/brightness?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.brightness = data.result;
        $scope.status = 'Loaded power';
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });

    $http.get('https://api.spark.io/v1/devices/' + $scope.deviceId + '/flipClock?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.flipClock = data.result;
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });

    $http.get('https://api.spark.io/v1/devices/' + $scope.deviceId + '/r?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.r = data.result;
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });

    $http.get('https://api.spark.io/v1/devices/' + $scope.deviceId + '/g?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.g = data.result;
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });

    $http.get('https://api.spark.io/v1/devices/' + $scope.deviceId + '/b?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.b = data.result;
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });

    $scope.getPatterns();

    $scope.disconnected = false;
  }

  $scope.getPower = function () {
    $scope.busy = true;
    $http.get('https://api.spark.io/v1/devices/' + $scope.deviceId + '/power?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.power = data.result;
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });
  };

  $scope.togglePower = function () {
    $scope.busy = true;
    var newPower = $scope.power == 0 ? 1 : 0;
    $http({
      method: 'POST',
      url: 'https://api.spark.io/v1/devices/' + $scope.deviceId + '/variable',
      data: { access_token: $scope.accessToken, args: "pwr:" + newPower },
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    }).
    success(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.power = data.return_value;
    }).
    error(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.status = data.error_description;
    });
  };

  $scope.toggleFlipClock = function () {
    $scope.busy = true;
    var newFlipClock = $scope.flipClock == 0 ? 1 : 0;
    $http({
      method: 'POST',
      url: 'https://api.spark.io/v1/devices/' + $scope.deviceId + '/variable',
      data: { access_token: $scope.accessToken, args: "flpclk:" + newFlipClock },
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    }).
    success(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.flipClock = data.return_value;
    }).
    error(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.status = data.error_description;
    });
  };

  $scope.getTimezone = function () {
    $scope.busy = true;
    $http.get('https://api.spark.io/v1/devices/' + $scope.deviceId + '/timezone?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.timezone = data.result;
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });
  };

  $scope.setTimezone = function ($) {
    $scope.busy = true;
    $http({
      method: 'POST',
      url: 'https://api.spark.io/v1/devices/' + $scope.deviceId + '/variable',
      data: { access_token: $scope.accessToken, args: "tz:" + $scope.timezone },
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    }).
    success(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.timezone = data.return_value;
    }).
    error(function (data, status, headers, config) {
      $scope.busy = false;
        $scope.status = data.error_description;
    });
  };

  $scope.getBrightness = function () {
    $scope.busy = true;
    $http.get('https://api.spark.io/v1/devices/' + $scope.deviceId + '/brightness?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.brightness = data.result;
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });
  };

  $scope.setBrightness = function ($) {
    $scope.busy = true;
    $http({
      method: 'POST',
      url: 'https://api.spark.io/v1/devices/' + $scope.deviceId + '/variable',
      data: { access_token: $scope.accessToken, args: "brt:" + $scope.brightness },
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    }).
    success(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.brightness = data.return_value;
    }).
    error(function (data, status, headers, config) {
      $scope.busy = false;
        $scope.status = data.error_description;
    });
  };

  $scope.setR = function ($) {
    $scope.busy = true;
    $http({
      method: 'POST',
      url: 'https://api.spark.io/v1/devices/' + $scope.deviceId + '/variable',
      data: { access_token: $scope.accessToken, args: "r:" + $scope.r },
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    }).
    success(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.r = data.return_value;
    }).
    error(function (data, status, headers, config) {
      $scope.busy = false;
        $scope.status = data.error_description;
    });
  };

  $scope.setG = function ($) {
    $scope.busy = true;
    $http({
      method: 'POST',
      url: 'https://api.spark.io/v1/devices/' + $scope.deviceId + '/variable',
      data: { access_token: $scope.accessToken, args: "g:" + $scope.g },
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    }).
    success(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.g = data.return_value;
    }).
    error(function (data, status, headers, config) {
      $scope.busy = false;
        $scope.status = data.error_description;
    });
  };

  $scope.setB = function ($) {
    $scope.busy = true;
    $http({
      method: 'POST',
      url: 'https://api.spark.io/v1/devices/' + $scope.deviceId + '/variable',
      data: { access_token: $scope.accessToken, args: "b:" + $scope.b },
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    }).
    success(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.b = data.return_value;
    }).
    error(function (data, status, headers, config) {
      $scope.busy = false;
        $scope.status = data.error_description;
    });
  };

  $scope.getPatternIndex = function () {
    $scope.busy = true;
    $http.get('https://api.spark.io/v1/devices/' + $scope.deviceId + '/patternIndex?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.patternIndex = data.result;
        $scope.pattern = $scope.patterns[$scope.patternIndex];
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });
  };

  $scope.getPatternNames = function (index) {
    if(index < $scope.patternCount) {
      var promise = patternService.getPatternName(index, $scope.deviceId, $scope.accessToken);
        promise.then(
          function(payload) {
            $scope.patterns.push( { index: index, name: payload.data.result });
            $scope.getPatternNames(index + 1);
          });
    }
    else {
      $scope.busy = false;
      $scope.getPatternIndex();
    }
  };

  $scope.getPatterns = function () {
    $scope.busy = true;

    // get the pattern count
    var promise = $http.get('https://api.spark.io/v1/devices/' + $scope.deviceId + '/patternCount?access_token=' + $scope.accessToken);

    // get the name of the first pattern
    // getPatternNames will then recursively call itself until all pattern names are retrieved
    promise.then(
      function (payload) {
        $scope.patternCount = payload.data.result;
        $scope.patterns = [];

        $scope.getPatternNames(0);
      },
      function (errorPayload) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });
  };

  $scope.setPattern = function() {
    $scope.busy = true;

    var promise = $http({
        method: 'POST',
        url: 'https://api.spark.io/v1/devices/' + $scope.deviceId + '/patternIndex',
        data: { access_token: $scope.accessToken, args: $scope.pattern.index },
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
      })
        .then(
          function (payload) {
            $scope.busy = false;
          },
          function (errorPayload) {
            $scope.busy = false;
          });
  };
});

app.factory('patternService', function ($http) {
  return {
    getPatternName: function (index, deviceId, accessToken) {
      return $http({
        method: 'POST',
        url: 'https://api.spark.io/v1/devices/' + deviceId + '/patternName',
        data: { access_token: accessToken, args: index },
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
      })
        .then(
        function (payload) {
          return $http.get('https://api.spark.io/v1/devices/' + deviceId + '/patternName?access_token=' + accessToken);
        });
    }
  }
});