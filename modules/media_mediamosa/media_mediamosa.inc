<?php
/**
 * @file
 * All non-hooks are put in this file.
 */

/**
 * Job.
 */
define('MEDIA_MEDIAMOSA_JOB_TYPE', 'job_type');
define('MEDIA_MEDIAMOSA_JOB_TYPE_TRANSCODE', 'TRANSCODE');
define('MEDIA_MEDIAMOSA_JOB_TYPE_STILL', 'STILL');

define('MEDIA_MEDIAMOSA_JOB_STATUS', 'status');
define('MEDIA_MEDIAMOSA_JOB_STATUS_FINISHED', 'FINISHED');
define('MEDIA_MEDIAMOSA_JOB_STATUS_FAILED', 'FAILED');
define('MEDIA_MEDIAMOSA_JOB_STATUS_WAITING', 'WAITING');

/**
 * Implements hook_form().
 *
 * @deprecated Use media_mediamosa_upload_form() instead.
 */
function _media_mediamosa_upload_form($form, &$form_state, $types = NULL) {
  return media_mediamosa_upload_form($form, $form_state, $types);
}

/**
 * Implements hook_form().
 *
 * Form callback for adding media via an upload form.
 */
function media_mediamosa_upload_form($form, &$form_state, $types = NULL) {

  $default_transcode_profile = variable_get('mediamosa_ck_transcode_profile_default');
  if (!isset($default_transcode_profile)) {
    $default_transcode_profile = MediaMosaCkConnectorWrapper::get_default_transcode_profile();
  }

  $media_mediamosa_transcodes = variable_get('mediamosa_ck_transcodes_autostart', array());
  if (!isset($media_mediamosa_transcodes)) {
    $media_mediamosa_transcodes = array($default_transcode_profile);
  }

  // Remove not choosen transcodes.
  $options['transcode'] = array();
  foreach ($media_mediamosa_transcodes as $value) {
    if ($value > 0) {
      $options['transcode'][] = $value;
    }
  }

  // Try to create upload ticket.
  $result = MediaMosaCkConnectorWrapper::create_upload_ticket($options);
  if (!$result) {
    $form['upload'] = array(
      '#markup' => t('Unable to upload to MediaMosa; @reason', array('@reason' => mediamosa_ck::get_connector_last_error_text())),
    );

    return $form;
  }

  // Save the upload ticket values to the form. This enables alter hooks to
  // access the data returned by create_upload_ticket().
  $form['media_mediamosa_upload_ticket'] = array(
    '#type' => 'value',
    '#value' => $result,
  );

  // Get upload URL.
  $upload_url = $result['action'];

  // Drupal 6;
  // <input type="file" name="file"  class="form-file required" id="edit-file"/>
  // Drupal 7
  // <input type="file" id="edit-file" name="files[file]" class="form-file" />
  //
  // Because we override.
  $form_state['has_file_element'] = TRUE;

  // FIXME: Hack to make it work with single array file upload to mediamosa.
  // To make it work with MediaMosa single array upload result.
  $form['file'] = array(
    '#markup' => '<input type="file" id="edit-file" name="file" size="60" class="form-file" />',
    '#title' => t('Upload a new file to MediaMosa'),
  );

  // Redirect value, we need to redirect back after upload so we can add it to
  // Drupal.
  $form['redirect_uri'] = array(
    '#type' => 'hidden',
    '#value' => url('media_mediamosa/mediafile/uploaded/' . rawurlencode($result['asset_id']) . '/' . rawurlencode($result['mediafile_id']), array('absolute' => TRUE)),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => '',
    '#attributes' => array('class' => array('button', 'fake-cancel')),
    '#weight' => 100,
  );

  $form['#mediamosa_upload_progress_id'] = $result['progress_id'];

  $form['#action'] = $upload_url;

  return $form;
}

/**
 * Access for media.
 */
function _media_mediamosa_media_permission($fid, $permission) {
  // File type check.
  if (!_media_mediamosa_file_type_check($fid)) {
    return FALSE;
  }

  return (empty($permission) || !user_access($permission)) ? FALSE : TRUE;
}

/**
 * File type check.
 *
 * @param int $fid
 *   File id.
 */
function _media_mediamosa_file_type_check($fid) {
  if (!is_numeric($fid)) {
    return FALSE;
  }

  // Again, we don't have media_load function, so we have to use
  // media_mediamosa_multi_load() and then reset array.
  $multi_media = media_mediamosa_multi_load($fid);

  if ($multi_media) {
    foreach ($multi_media as $media) {
      if (!isset($media->uri)) {
        return FALSE;
      }
    }
  }
  else {
    return FALSE;
  }

  $uri = explode('://', $media->uri);

  return $uri[0] == 'mediamosa';
}

/**
 * Get running jobs and create a form item for that information.
 *
 * @param array &$form
 *   The form to add to.
 * @param string $mediafile_id
 *   The mediafile_id of the item to get jobs from.
 * @param string $job_type
 *   The job type to show.
 */
function _media_mediamosa_get_running_jobs(&$form, $mediafile_id, $job_type) {
  $jobs = MediaMosaCkConnectorWrapper::get_mediafile_jobs($mediafile_id, mediamosa_ck::session_user_id());
  $jobs_count = 0;
  if ($jobs->items->item) {
    foreach ($jobs->items->item as $job) {
      if ((string) $job->{MEDIA_MEDIAMOSA_JOB_STATUS} != MEDIA_MEDIAMOSA_JOB_STATUS_FINISHED && (string) $job->{MEDIA_MEDIAMOSA_JOB_STATUS} != MEDIA_MEDIAMOSA_JOB_STATUS_FAILED && (string) $job->{MEDIA_MEDIAMOSA_JOB_TYPE} == $job_type) {
        $jobs_count++;
      }
    }
  }
  if ($jobs_count) {
    $form['running_jobs'] = array(
      '#type' => 'item',
      '#title' => t(
        'Running @job_type jobs',
        array(
          '@job_type' => drupal_strtolower($job_type),
        )
      ),
      '#markup' => format_plural(
        $jobs_count,
        'There is @count @job_type job in progress.',
        'There are @count @job_type jobs in progress.',
        array('@job_type' => drupal_strtolower($job_type))
      ),
    );
  }
}

/**
 * Return the time in RFC 3339 format.
 *
 * Eg. 2003-12-13T18:30:02Z
 * @link http://www.ietf.org/rfc/rfc3339.txt RFC 3339
 *
 * @param datetime $time
 *   The time. If not given, the application uses the current time.
 *
 * @return string
 *   The formatted string.
 */
function _media_mediamosa_time_to_rfc_3339($time = NULL) {
  return $time ? gmdate('Y-m-d\TH:i:s\Z', $time) : gmdate('Y-m-d\TH:i:s\Z');
}

/**
 * Delete the transcoded mediafile of an asset.
 *
 * @param string $asset_id
 *   asset id.
 *
 * @return array
 *   Returns an array with the mediafile IDs that where deleted.
 */
function _media_mediamosa_delete_mediafile_transcoded($asset_id) {
  $mediafile_ids = array();

  $asset = MediaMosaCkConnectorWrapper::get_asset($asset_id);

  if (isset($asset->items->item->mediafiles->mediafile) && $asset->items->item->mediafiles->mediafile) {
    foreach ($asset->items->item->mediafiles->mediafile as $mediafile) {
      if ((string) $mediafile->is_original_file != 'TRUE') {
        MediaMosaCkConnectorWrapper::delete_mediafile((string) $mediafile->mediafile_id, (string) $mediafile->owner_id);
      }
    }
  }

  return $mediafile_ids;
}
