var app = angular.module('app', []);

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
  $scope.color = "#0000ff"
  $scope.r = 0;
  $scope.g = 0;
  $scope.b = 255;
  $scope.powerText = "On";
  $scope.status = "Please enter your access token.";
  $scope.disconnected = false;

  $scope.patterns = [];
  $scope.patternIndex = 0;

  $scope.devices = [];

  $scope.status = 'Loading access token...';
  chrome.storage.sync.get('accessToken',
    function(result) {
      $scope.status = 'Loaded access token';
      $scope.accessToken = result.accessToken;
      $('#inputAccessToken').scope().$apply();
    }
  );

  $scope.getDevices = function() {
    $scope.status = 'Getting devices...';

    $http.get('https://api.spark.io/v1/devices?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.devices = data;
        if(data && data.length > 0)
          $scope.device = data[0];
        $scope.status = 'Loaded devices';
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });
  }

  $scope.save = function () {
    $scope.status = 'Saving access token...';
    chrome.storage.sync.set({'accessToken': $scope.accessToken},
    function() {
      $scope.status = 'Saved access token';
    });
  }

  $scope.connect = function() {
    $scope.busy = true;

    $http.get('https://api.spark.io/v1/devices/' + $scope.device.id + '/power?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.power = data.result;
        $scope.status = 'Loaded power';
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });

    $http.get('https://api.spark.io/v1/devices/' + $scope.device.id + '/timezone?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.timezone = data.result;
        $scope.status = 'Loaded time zone';
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });

    $http.get('https://api.spark.io/v1/devices/' + $scope.device.id + '/brightness?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.brightness = data.result;
        $scope.status = 'Loaded brightness';
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });

    $http.get('https://api.spark.io/v1/devices/' + $scope.device.id + '/flipClock?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.flipClock = data.result;
        $scope.status = 'Loaded clock orientation';
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });

    $http.get('https://api.spark.io/v1/devices/' + $scope.device.id + '/r?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.r = data.result;
        $scope.status = 'Loaded red';
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });

    $http.get('https://api.spark.io/v1/devices/' + $scope.device.id + '/g?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.g = data.result;
        $scope.status = 'Loaded green';
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });

    $http.get('https://api.spark.io/v1/devices/' + $scope.device.id + '/b?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.b = data.result;
        $scope.status = 'Loaded blue';
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });

    $scope.getPatterns();
  }

  $scope.getPower = function () {
    $scope.busy = true;
    $http.get('https://api.spark.io/v1/devices/' + $scope.device.id + '/power?access_token=' + $scope.accessToken).
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
      url: 'https://api.spark.io/v1/devices/' + $scope.device.id + '/variable',
      data: { access_token: $scope.accessToken, args: "pwr:" + newPower },
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    }).
    success(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.power = data.return_value;
      $scope.status = $scope.power == 1 ? 'Turned on' : 'Turned off';
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
      url: 'https://api.spark.io/v1/devices/' + $scope.device.id + '/variable',
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
    $http.get('https://api.spark.io/v1/devices/' + $scope.device.id + '/timezone?access_token=' + $scope.accessToken).
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
      url: 'https://api.spark.io/v1/devices/' + $scope.device.id + '/variable',
      data: { access_token: $scope.accessToken, args: "tz:" + $scope.timezone },
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    }).
    success(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.timezone = data.return_value;
      $scope.status = 'Time zone set';
    }).
    error(function (data, status, headers, config) {
      $scope.busy = false;
        $scope.status = data.error_description;
    });
  };

  $scope.getBrightness = function () {
    $scope.busy = true;
    $http.get('https://api.spark.io/v1/devices/' + $scope.device.id + '/brightness?access_token=' + $scope.accessToken).
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
      url: 'https://api.spark.io/v1/devices/' + $scope.device.id + '/variable',
      data: { access_token: $scope.accessToken, args: "brt:" + $scope.brightness },
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    }).
    success(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.brightness = data.return_value;
      $scope.status = 'Brightness set';
    }).
    error(function (data, status, headers, config) {
      $scope.busy = false;
        $scope.status = data.error_description;
    });
  };

  $scope.setColor = function ($) {
    var rgb = $scope.hexToRgb();

    $scope.r = rgb.r;
    $scope.g = rgb.g;
    $scope.b = rgb.b;

    $scope.setR();
    $scope.setG();
    $scope.setB();
  };

  $scope.hexToRgb = function ($) {
    var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec($scope.color);
    return result ? {
        r: parseInt(result[1], 16),
        g: parseInt(result[2], 16),
        b: parseInt(result[3], 16)
    } : null;
  }

  $scope.setR = function ($) {
    $scope.busy = true;
    $http({
      method: 'POST',
      url: 'https://api.spark.io/v1/devices/' + $scope.device.id + '/variable',
      data: { access_token: $scope.accessToken, args: "r:" + $scope.r },
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    }).
    success(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.r = data.return_value;
      $scope.status = 'Red set';
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
      url: 'https://api.spark.io/v1/devices/' + $scope.device.id + '/variable',
      data: { access_token: $scope.accessToken, args: "g:" + $scope.g },
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    }).
    success(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.g = data.return_value;
      $scope.status = 'Green set';
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
      url: 'https://api.spark.io/v1/devices/' + $scope.device.id + '/variable',
      data: { access_token: $scope.accessToken, args: "b:" + $scope.b },
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    }).
    success(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.b = data.return_value;
      $scope.status = 'Blue set';
    }).
    error(function (data, status, headers, config) {
      $scope.busy = false;
        $scope.status = data.error_description;
    });
  };

  $scope.getPatternIndex = function () {
    $scope.busy = true;
    $http.get('https://api.spark.io/v1/devices/' + $scope.device.id + '/patternIndex?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.patternIndex = data.result;
        $scope.pattern = $scope.patterns[$scope.patternIndex];
        $scope.disconnected = false;
        $scope.status = 'Ready';
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });
  };

  $scope.getPatternNames = function (index) {
    if(index < $scope.patternCount) {
      var promise = patternService.getPatternName(index, $scope.device.id, $scope.accessToken);
        promise.then(
          function(payload) {
            $scope.patterns.push( { index: index, name: payload.data.result });
            $scope.status = 'Loaded pattern name ' + index;
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
    var promise = $http.get('https://api.spark.io/v1/devices/' + $scope.device.id + '/patternCount?access_token=' + $scope.accessToken);

    // get the name of the first pattern
    // getPatternNames will then recursively call itself until all pattern names are retrieved
    promise.then(
      function (payload) {
        $scope.patternCount = payload.data.result;
        $scope.patterns = [];
        $scope.status = 'Loaded pattern count';

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
        url: 'https://api.spark.io/v1/devices/' + $scope.device.id + '/patternIndex',
        data: { access_token: $scope.accessToken, args: $scope.pattern.index },
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
      })
        .then(
          function (payload) {
            $scope.busy = false;
            $scope.status = 'Pattern set';
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