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
  $scope.brightness = 64;
  $scope.deviceId = "";
  $scope.accessToken = "";
  $scope.busy = false;
  $scope.pattern = {};
  $scope.power = 1;
  $scope.powerText = "On";
  $scope.status = "Please enter your device ID and access token.";

  $scope.patterns = [];

  chrome.storage.sync.get('deviceId',
    function(result) {
      $scope.deviceId = result.deviceId;
    }
  );

  chrome.storage.sync.get('accessToken',
    function(result) {
      $scope.accessToken = result.accessToken;
    }
  );

  $scope.save = function () {
    chrome.storage.sync.set({'deviceId': $scope.deviceId, 'accessToken': $scope.accessToken},
    function() {
      message('Settings saved');
    });
  }

  $scope.getPower = function () {
    $scope.busy = true;
    $http.get('https://api.spark.io/v1/devices/' + $scope.deviceId + '/power?access_token=' + $scope.accessToken).
      success(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.power = data.result;
        $scope.powerText = $scope.power == 1 ? "On" : "Off";
      }).
      error(function (data, status, headers, config) {
        $scope.busy = false;
        $scope.status = data.error_description;
      });
  };

  $scope.setPower = function (power) {
    $scope.busy = true;
    $http({
      method: 'POST',
      url: 'https://api.spark.io/v1/devices/' + $scope.deviceId + '/power',
      data: { access_token: $scope.accessToken, args: power },
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    }).
    success(function (data, status, headers, config) {
      $scope.busy = false;
      $scope.power = data.return_value;
      $scope.powerText = $scope.power == 1 ? "On" : "Off";
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
      url: 'https://api.spark.io/v1/devices/' + $scope.deviceId + '/brightness',
      data: { access_token: $scope.accessToken, args: $scope.brightness },
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