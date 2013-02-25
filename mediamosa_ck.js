/**
 * Basic JS functions.
 */

(function($) {

Drupal.mediamosaCK = Drupal.mediamosaCK || {};

/**
 * Create upload ticket.
 *
 * @return array
 *   - 'action'
 *     The upload URL for the file.
 *   - 'progress_id'
 *     Progress ID to use when getting upload progression from server.
 *   - 'asset_id'
 *     The asset ID of the upload.
 *   - 'mediafile_id'
 *     The mediafile ID of the upload.
 */
Drupal.mediamosaCK.getUploadTicket = function() {
  var result = null;
  $.ajax({
    type: 'GET',
    url: Drupal.settings.basePath + 'mediamosa/ck/json/uploadticket/create',
		async: false,
    dataType: 'json',
    success: function (uploadticket) {
      if (typeof uploadticket.action != 'undefined') {
        result = uploadticket.action;
        console.log('Got upload ticket;' + uploadticket.action);
      }
    },
    error: function (xmlhttp) {
      alert(Drupal.ajaxError(xmlhttp, 'mediamosa/ck/json/uploadticket/create'));
      console.log('Unable to get upload ticket!');
    }
  });
  return result;
}

/**
 * Check if MediaMosa connector is setup correctly.
 */
Drupal.mediamosaCK.getConnectorStatus = function() {
  var result = false;
  $.ajax({
    type: 'GET',
    url: Drupal.settings.basePath + 'mediamosa/ck/json/connector/status',
		async: false,
    dataType: 'json',
    success: function (status) {
      result = (status.ok == 1);
    },
    error: function (xmlhttp) {
      alert(Drupal.ajaxError(xmlhttp, 'mediamosa/ck/json/connector/status'));
    }
  });
  return result;
}

Drupal.behaviors.mediamosa_ck_tabs = {
  attach: function (context, settings) {
    var navigation = $('#mediamosa-ck-tabs li');

    // Remove the active state of the tabs if there are any.
    var tabs = $('.mediamosa-ck-tabs .mediamosa-ck-tab');
    tabs.hide();

    // On page load, set first tab active and set the first content field on
    // active.
    $('#mediamosa-ck-tabs li:first').addClass('active');
    var defaultActiveContent = $('#mediamosa-ck-tabs li:first a').attr('name');
    $('#' + defaultActiveContent).show();

    navigation.find('a').click(function(event){
      tabs.hide();
      navigation.removeClass('active');
      $('#' + $(this).attr('name')).toggle();
      $(this).parent('li').addClass('active');
    })
  }
};

})(jQuery);

/**
<div class="mediamosa-ck-tabs">
  <ul id="mediamosa-ck-tabs">
    <li>
      <?php print l('Description', '' . $fields['asset_id']->raw, array('fragment' => 'tab-#name#', 'attributes' => array('name' => 'tab-#name#'))); ?>
    </li>
  </ul>

  <div class="tab" id="tab-#name#">
    <h2>Metadata DC</h2>
    <?php print $content; ?>
  </div>
</div>


<div class="mediamosa-ck-tabs">
  <ul id="mediamosa-ck-tabs">
    <li>
      <a href="/asset/detail/BhYmBNCkFQUSnXiQVxVuQBsh#tab-metadata-dc" name="tab-metadata-dc" class="active">Description</a>    </li>
    <li>
      <a href="/asset/detail/BhYmBNCkFQUSnXiQVxVuQBsh#tab-metadata-qdc" name="tab-metadata-qdc" class="active">More info</a>    </li>
    <li>
      <a href="/asset/detail/BhYmBNCkFQUSnXiQVxVuQBsh#tab-technical-metadata" name="tab-technical-metadata" class="active">Technical info</a>    </li>
  </ul>

  <div class="tab" id="tab-metadata-dc" style="display: none;">
    <h2>Metadata DC</h2>
    <span class="field-content"><table>
<tbody>
 <tr class="odd"><td>Contributor</td><td> </td> </tr>
 <tr class="even"><td>Coverage spatial</td><td> </td> </tr>
 <tr class="odd"><td>Coverage temporal</td><td> </td> </tr>
 <tr class="even"><td>Creator</td><td> </td> </tr>
 <tr class="odd"><td>Date</td><td> </td> </tr>
 <tr class="even"><td>Description</td><td> </td> </tr>
 <tr class="odd"><td>Format</td><td> </td> </tr>
 <tr class="even"><td>Identifier</td><td> </td> </tr>
 <tr class="odd"><td>Language</td><td> </td> </tr>
 <tr class="even"><td>Publisher</td><td> </td> </tr>
 <tr class="odd"><td>Relation</td><td> </td> </tr>
 <tr class="even"><td>Rights</td><td> </td> </tr>
 <tr class="odd"><td>Source</td><td> </td> </tr>
 <tr class="even"><td>Subject</td><td> </td> </tr>
 <tr class="odd"><td>Title</td><td> </td> </tr>
 <tr class="even"><td>Type</td><td> </td> </tr>
</tbody>
</table>
</span>  </div>

  <div class="tab" id="tab-metadata-qdc" style="display: none;">
    <h2>Metadata QDC</h2>
    <span class="field-content"><table>
<tbody>
 <tr class="odd"><td>Created</td><td> </td> </tr>
 <tr class="even"><td>Description abstract</td><td> </td> </tr>
 <tr class="odd"><td>Format extent</td><td> </td> </tr>
 <tr class="even"><td>Format medium</td><td> </td> </tr>
 <tr class="odd"><td>Hasformat</td><td> </td> </tr>
 <tr class="even"><td>Isformatof</td><td> </td> </tr>
 <tr class="odd"><td>Isreferencedby</td><td> </td> </tr>
 <tr class="even"><td>Issued</td><td> </td> </tr>
 <tr class="odd"><td>License</td><td> </td> </tr>
 <tr class="even"><td>Rightsholder</td><td> </td> </tr>
 <tr class="odd"><td>Title alternative</td><td> </td> </tr>
</tbody>
</table>
</span>  </div>

  <div class="tab" id="tab-technical-metadata" style="display: block;">
    <h2>Technical metadata</h2>
    <span class="field-content"><table>
<tbody>
 <tr class="odd"><td>Audio codec</td><td>flac</td> </tr>
 <tr class="even"><td>Bitrate</td><td>840</td> </tr>
 <tr class="odd"><td>Bpp</td><td>0.6</td> </tr>
 <tr class="even"><td>Channels</td><td>2</td> </tr>
 <tr class="odd"><td>Colorspace</td><td>yuv420p</td> </tr>
 <tr class="even"><td>Container type</td><td>ogg</td> </tr>
 <tr class="odd"><td>File duration</td><td>00:00:03.65</td> </tr>
 <tr class="even"><td>Filesize</td><td>384414</td> </tr>
 <tr class="odd"><td>Fps</td><td>25</td> </tr>
 <tr class="even"><td>Height</td><td>176</td> </tr>
 <tr class="odd"><td>Is hinted</td><td>FALSE</td> </tr>
 <tr class="even"><td>Is inserted md</td><td>FALSE</td> </tr>
 <tr class="odd"><td>Md5</td><td>c24f395648ebf23e605079426cb70e23</td> </tr>
 <tr class="even"><td>Mime type</td><td>application/ogg</td> </tr>
 <tr class="odd"><td>Sample rate</td><td>44100</td> </tr>
 <tr class="even"><td>Video codec</td><td>theora</td> </tr>
 <tr class="odd"><td>Width</td><td>320</td> </tr>
</tbody>
</table>
</span>  </div>
</div>
* */