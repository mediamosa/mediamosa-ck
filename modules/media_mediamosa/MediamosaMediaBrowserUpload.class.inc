<?php
/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * Media browser internet plugin, displays the upload form.
 */
class MediamosaMediaBrowserUpload extends MediaBrowserPlugin {

  function view() {
    $from_web_form = drupal_get_form('_media_mediamosa_upload_form',  $this->params['types'], $this->params['multiselect']);
    return array(
      'form' => array($from_web_form),
      '#attached' => array(
        //'js' => array($path . '/js/plugins/media.fromurl.js'),
      ),
    );
  }
}
