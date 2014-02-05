/*
TO USE: include reloadr.js and tell it what to check:

  Reloadr.go({
    client: [
      '/js/main.js',
      '/css/layout.css'
    ],
    server: [
      'index.php'
    ],
    path: '/reloadr.php',
    frequency: 2000
  });

  All keys are optional. If you don't give client or server, though, it won't do anything.

  Shortcut: Reloadr.watch([
    '/js/main.js',
    '/css/layout.css'
  ]);
*/


var Reloadr = {
  req:     new XMLHttpRequest(),
  timeout: null,
  options: {
    client:    [],
    frequency: 2000,
    server:    [],
    path:      '/reloadr.php'
  },

  // ---------------------------------------------------------------------------

  disable: function () { localStorage.setItem('reloadrDisabled', 'true'); },
  enable:  function () { localStorage.setItem('reloadrDisabled', 'false'); },
  init:    function () { window._Reloadr_LoadTime = new Date(); },
  watch:   function (options) { this.go.call(this, options); },

  //----------------------------------------------------------------------------

  ajax: function (url, callback) {
    if (localStorage.getItem('reloadrDisabled') === 'true') {
      return;
    }

    this.req.open('GET', url, false);
    this.req.setRequestHeader('If-Modified-Since', window._Reloadr_LoadTime.toUTCString());
    this.req.send(null);

    if (this.req.status == 200) {
      callback.call(Date.parse(this.req.getResponseHeader('Last-Modified')));
    }
  },

  // ---------------------------------------------------------------------------

  go: function (options) {
    if (options) {
      // Deal with array being passed.
      if (typeof options.length != 'undefined') {
        this.options.client = options;
      }
      else {
        // Change any options given.
        for (x in options) {
          this.options[x] = options[x];
        }
      }
    }

    // Set up new timeout.
    clearTimeout(this.timeout);
    this.timeout = setTimeout(function () {
      Reloadr.poll.call(Reloadr);
    }, this.options.frequency);
  },

  // ---------------------------------------------------------------------------

  poll: function (options) {
    var urls = this.options.client.slice();

    // Build URL for server-side files.
    if (this.options.server.length) {
      urls.push(this.options.path + '?' + this.options.server.join(','));
    }

    var self = this;

    // Check the URLs.
    for (i in urls) {
      (function (i) {
        self.ajax.call(self, urls[i], function () {
          if (this > Date.parse(window._Reloadr_LoadTime)) {
            if (urls[i].slice(-3) === 'css') {
              // Handle linked CSS.
              var links = document.getElementsByTagName('link');
              for (j = 0; j < links.length; j++) {
                var link = links[j];
                if (link.rel === 'stylesheet' && link.href.indexOf(urls[i]) > -1) {
                  link.href = link.href.replace(/(.*)\.css(.*)/gi, '$1.css?' + (+new Date));
                }
              }

              // Handle imported CSS.
              var styles = document.styleSheets;
              for (var j = 0; j < styles.length; j++) {
                var style = styles[j];
                var rules = style.cssRules || style.rules;
                if (rules) {
                  for (var k = 0; k < rules.length; k++) {
                    if (rules[k] instanceof CSSImportRule) {
                      var rule_css = rules[k].cssText;
                      if (rule_css.indexOf(urls[i]) > -1) {
                        var new_rule_css = rule_css.replace(/@import(.*)\.css(.*)"\)/i, '@import$1.css?' + (+new Date) + '")');
                        style.deleteRule(k);
                        style.insertRule(new_rule_css, k);
                      }
                    }
                  }
                }
              }

              window._Reloadr_LoadTime = new Date();
            }
            else {
              location.reload();
            }
          }
        });
      })(i);
    }

    this.go();
  }
};

Reloadr.init();
