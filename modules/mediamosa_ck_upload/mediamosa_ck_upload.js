(function($) {

Drupal.mediamosaCK = Drupal.mediamosaCK || {};

Drupal.mediamosaCK.refreshUploadlist = function() {
  if ($('#edit-mediamosa-ck-upload-listing-refresh') !== undefined) {
    jQuery($('#edit-mediamosa-ck-upload-listing-refresh')).trigger('click');
  }
};

/**
 * Process uploaded file.
 *
 * @param file file
 * @returns {uploadticket|Drupal.mediamosaCK.fileUploaded.result}
 */

Drupal.mediamosaCK.fileUploaded = function(file) {
  var result = null;
  $.ajax({
    type: 'GET',
    url: Drupal.settings.basePath + 'mediamosa/ck/json/upload/fileuploaded/' + escape(file.mediafile_id),
		async: false,
    dataType: 'json',
    success: function (result) {
        $(document).trigger("mediamosa_file_uploaded",file.asset_id);
    },
    error: function (xmlhttp) {
      alert(Drupal.ajaxError(xmlhttp, 'mediamosa/ck/json/upload/fileuploaded'));
      console.log('Unable to process post file upload.');
    }
  });
};

/**
 * Attaches the Plupload behavior to each Plupload form element.
 */
Drupal.behaviors.ckupload = {
  attach: function (context, settings) {
    // Refresh.
    Drupal.mediamosaCK.refreshUploadlist();

    $(".mm-ck-upload-element", context).once('plupload-init', function () {
      var $this = $(this);

      // Merge the default settings and the element settings to get a full
      // settings object to pass to the Plupload library for this element.
      var id = $this.attr('id');
      var defaultSettings = settings.plupload['_default'] ? settings.plupload['_default'] : {};
      var elementSettings = (id && settings.plupload[id]) ? settings.plupload[id] : {};
      var pluploadSettings = $.extend({}, defaultSettings, elementSettings);

      // Process Plupload events.
      if (elementSettings['init'] || false) {
        if (!pluploadSettings.init) {
          pluploadSettings.init = {};
        }
        for (var key in elementSettings['init']) {
          var callback = elementSettings['init'][key].split('.');
          var fn = window;
          for (var j = 0; j < callback.length; j++) {
            fn = fn[callback[j]];
          }
          if (typeof fn === 'function') {
            pluploadSettings.init[key] = fn;
          }
        }
      }

      // Initialize Plupload for this element.
      $this.pluploadQueue(pluploadSettings);
    });
  }
};

/**
 * Attaches the Plupload behavior to each Plupload form element.
 */
Drupal.behaviors.ckuploadform = {
  attach: function(context, settings) {
    $('form', context).once('plupload-form', function() {
      if (0 < $(this).find('.mm-ck-upload-element').length) {
        var $form = $(this);
        var originalFormAttributes = {
          'method': $form.attr('method'),
          'enctype': $form.attr('enctype'),
          'action': $form.attr('action'),
          'target': $form.attr('target')
        };

        if (Drupal.mediamosaCK.getConnectorStatus() !== true) {
          alert(Drupal.t('MediaMosa connector is not setup.'));
        }

        $(this).find('.mm-ck-upload-element').each( function(index) {
          var uploader = $(this).pluploadQueue();
          uploader.bind('BeforeUpload', function(Uploader, file) {
            var uploadticket = Drupal.mediamosaCK.getUploadTicket(file);
            Uploader.settings['url'] = uploadticket.action;
            file['asset_id'] = uploadticket.asset_id;
            file['mediafile_id'] = uploadticket.mediafile_id;
          });

          uploader.bind('FileUploaded', function(Uploader, file) {
            Drupal.mediamosaCK.refreshUploadlist();
            Drupal.mediamosaCK.fileUploaded(file)
          });
        });

        $(this).submit(function(e) {
          var completedPluploaders = 0;
          var totalPluploaders = $(this).find('.mm-ck-upload-element').length;
          var errors = '';

          $(this).find('.mm-ck-upload-element').each( function(index) {
            var uploader = $(this).pluploadQueue();

            var id = $(this).attr('id');
            var defaultSettings = settings.plupload['_default'] ? settings.plupload['_default'] : {};
            var elementSettings = (id && settings.plupload[id]) ? settings.plupload[id] : {};
            var pluploadSettings = $.extend({}, defaultSettings, elementSettings);

            // Only allow the submit to proceed if there are files and they've
            // all completed uploading.
            if (uploader.state == plupload.STARTED) {
              errors += Drupal.t("Please wait while your files are being uploaded.");
            }
            else if (uploader.files.length == 0 && !pluploadSettings.required) {
              completedPluploaders++;
            }
            else if (uploader.files.length == 0) {
              errors += Drupal.t("@index: You must upload at least one file.\n",{'@index': (index + 1)});
            }
            else if (uploader.files.length > 0 && uploader.total.uploaded == uploader.files.length) {
              completedPluploaders++;
            }
            else {
              var stateChangedHandler = function() {
                if (uploader.total.uploaded == uploader.files.length) {
                  uploader.unbind('StateChanged', stateChangedHandler);
                  completedPluploaders++;
                  if (completedPluploaders == totalPluploaders ) {
                    for (var attr in originalFormAttributes) {
                      $form.attr(attr, originalFormAttributes[attr]);
                    }
                    $form.submit();
                    return true;
                  }
                }
              };
              uploader.bind('StateChanged', stateChangedHandler);
              uploader.start();
            }
          });

          if (completedPluploaders == totalPluploaders) {
            for (var attr in originalFormAttributes) {
              $form.attr(attr, originalFormAttributes[attr]);
            }
            return true;
          }
          else if (0 < errors.length) {
            alert(errors);
          }

          return false;
        });
      }
    });
  }
};

/**
 * Helper function to compare version strings.
 *
 * Returns one of:
 *   - A negative integer if a < b.
 *   - A positive integer if a > b.
 *   - 0 if a == b.
 */
Drupal.mediamosaCK.compareVersions = function (a, b) {
  a = a.split('.');
  b = b.split('.');
  // Return the most significant difference, if there is one.
  for (var i=0; i < Math.min(a.length, b.length); i++) {
    var compare = parseInt(a[i]) - parseInt(b[i]);
    if (compare !== 0) {
      return compare;
    }
  }
  // If the common parts of the two version strings are equal, the greater
  // version number is the one with the most sections.
  return a.length - b.length;
}

})(jQuery);
