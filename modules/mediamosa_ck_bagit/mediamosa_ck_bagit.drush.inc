<?php
/**
 * @file
 * Drush integration for MediaMosa Bagit.
 *
 * Contains drush bagit related functions.
 */

/**
 * Implements hook_drush_command().
 */
function mediamosa_ck_bagit_drush_command() {
  $items = array();

  $items['mm-import-bagit'] = array(
    'description' => dt('Imports an BagIt into an asset.'),
    'aliases' => array('mmimpb'),
    'arguments' => array(
      'path' => 'Path where the BagIt can be found.',
      'application id' => 'app_id in which the bagit is imported, defaults to 1.',
      'owner' => 'Name of the owner to be stored in the asset(s).',
    ),
    'options' => array(
      'r' => 'recursive. Will treat every directory in path as a seperate Bagit.',
      'quiet' => 'Suppress messages.',
      'maintain_ids' => FALSE,
    ),
  );
  $items['mm-export-bagit'] = array(
    'description' => dt('Exports an asset to a BagIt.'),
    'aliases' => array('mmexpb'),
    'arguments' => array(
      'asset_id' => 'Asset id to be exported.',
      'path' => 'Path where the BagIt is stored.',
    ),
    'options' => array(
      'quiet' => 'Suppress messages.',
    ),
    'examples' => array(
      'drush mm-export-bagit /tmp --quiet' => 'Exports asset LuBaPJcWIRUXaJTlQgQJElV2 to a directory in /tmp/.',
    ),
  );
  $items['mm-export-bagit-app'] = array(
    'description' => dt('Exports all assets of an app to BagIts.'),
    'aliases' => array('mmexpbapp'),
    'arguments' => array(
      'app_id' => 'App id to be exported.',
      'path' => 'Path where the BagIt is stored.',
    ),
    'options' => array(
      'quiet' => 'Suppress messages.',
    ),
    'examples' => array(
      'drush mm-export-bagit-app 1 /tmp --quiet' => 'Exports all assets of app id 1 to directory /tmp.',
    ),
  );
  $items['mm-import-bagit-dir'] = array(
    'description' => dt('Imports a directory of BagIts into an application.'),
    'aliases' => array('mmimpbdir'),
    'arguments' => array(
      'application id' => 'app_id in which the bagit is imported.',
      'path' => 'Path where the BagIt can be found.',
      'owner' => 'Name of the owner to be stored in the asset(s).',
    ),
    'options' => array(
      'quiet' => 'Suppress messages.',
      'maintain_ids' => 'Maintain asset_ids (if present in bag).',
    ),
  );
  return $items;
}

/**
 * Helper function to handle non standard metadata definitions.
 */
function _mediamosa_ck_bagit_metadatagroup_convert($tag_name, $to_bagit = TRUE) {
  $conversions = array(
    'dublin_core' => 'DC',
    'qualified_dublin_core' => 'QDC',
  );
  if ($to_bagit) {
    // Convert from MediaMosa to BagIt.
    return (array_search($tag_name, $conversions));
  }
  else {
    // Convert from BagIt to MediaMosa.
    if (isset($conversions[$tag_name])) {
      return ($conversions[$tag_name]);
    }
  }
  return $tag_name;
}

/**
 * Helper function to handle non standard metadata definitions.
 */
function _mediamosa_ck_bagit_metadata_convert($tag_name, $to_bagit = TRUE) {
  $conversions = array(
    'dc-accessrights' => 'rights',
    'dc-dateaccepted' => 'date',
    'qdc-date-issued' => 'issued',
    'qdc-date_issued' => 'issued',
  );
  $metadatagroups = array('dublin_core' => 'dc-', 'qualified_dublin_core' => 'qdc-');
  $metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full(FALSE, array('dublin_core', 'qualified_dublin_core'));
  foreach ($metadata_definitions_full as $prop => $propinfo) {
    $conversions[$metadatagroups[$propinfo['propgroup_name']] . $prop] = $prop;
  }

  if ($to_bagit) {
    // Convert from MediaMosa to BagIt.
    return (array_search($tag_name, $conversions));
  }
  else {
    // Convert from BagIt to MediaMosa.
    $tag_name = strtolower($tag_name);
    if (isset($conversions[$tag_name])) {
      return ($conversions[$tag_name]);
    }
  }
  return $tag_name;
}

/**
 * Callback function for mm-import-bagit command.
 *
 * Import Bagit into an asset.
 *
 * @todo: If an reference id is found and is present, it will overwrite the
 * existing asset.
 */
function drush_mediamosa_mm_import_bagit($path, $app_id = 1, $owner = 'N.N.') {

  // Sanitize path.
  $path .= (drupal_substr($path, -1) == '/' ? '' : '/');

  // $symlink = drush_get_option('symlink', NULL);
  $quiet = drush_get_option('quiet', NULL);
  drush_set_option('quiet', TRUE);
  // $skip_bagit_validation = drush_get_option('skip_bagit_validation', NULL);
  $maintain_ids = drush_get_option('maintain_ids', FALSE);

  if (!is_numeric($app_id)) {
    drush_print('Invalid app_id.');
    return FALSE;
  }

  if (count(scandir($path)) == 2) {
    if (!$quiet) {
      drush_print('No files found in directory.');
    }
    else {
      return FALSE;
    }
  }
  // @todo: if $path contains no baginfo.txt give warning.
  // @todo: if $path is a zip/tgz file, use that one.

  $bagitlib = libraries_detect('bagitphp');
  if ($bagitlib && !empty($bagitlib['installed'])) {
    libraries_load('bagitphp');
  }
  else {
    drupal_set_message('No bagit library detected.', 'error');
    return;
  }

  $bag = new BagIt($path);

  // Validate the bag.
  $bag->validate();

  // Only execute if a valid bag.
  if ((count($bag->getBagErrors()) == 0) && (!empty($bag->bagInfoData))) {

    // Parse bag-info.txt Metadata and store it.
    $metadata = array();
    foreach ($bag->bagInfoData as $prop => $value) {
      if (isset($value) && (!empty($value))) {
        if (is_array($value)) {
          $metadata[_mediamosa_ck_bagit_metadata_convert($prop, FALSE)] = $value;
        }
        else {
          $metadata[_mediamosa_ck_bagit_metadata_convert($prop, FALSE)][] = $value;
        }
      }
    }

    // Owner.
    $owner = isset($metadata['mediamosa-owner-id']) ? $metadata['mediamosa-owner-id'] : $owner;

    // Use the asset-id from the bag-info, if given.
    if (isset($metadata['mediamosa-asset-id'][0]) && ($maintain_ids == TRUE)) {
      $asset_id = trim($metadata['mediamosa-asset-id'][0]);
      if (!mediamosa_asset::get($asset_id)) {
        $asset_id = mediamosa_asset::create($app_id, $owner, NULL, NULL, NULL, TRUE, FALSE, $asset_id);
      }
    }
    $update_asset = FALSE;
    if (!isset($asset_id)) {
      // Create a new asset id.
      $asset_id = mediamosa_asset::create($app_id, NULL, $owner, NULL);
    }
    else {
      $update_asset = TRUE;
    }

    // Now update general asset info.
    mediamosa_asset::update(
      $app_id,
      NULL,
      TRUE,
      $asset_id,
      $owner,
      NULL,
      isset($metadata['mediamosa-play-restriction-start']) ? $metadata['mediamosa-play-restriction-start'] : NULL,
      isset($metadata['mediamosa-play-restriction-end']) ? $metadata['mediamosa-play-restriction-end'] : NULL,
      isset($metadata['mediamosa-isprivate']),
      NULL,
      isset($metadata['mediamosa-reference-id']) ? $metadata['mediamosa-reference-id'] : NULL,
      isset($metadata['mediamosa-provider-id']) ? $metadata['mediamosa-provider-id'] : NULL,
      isset($metadata['mediamosa-asset-type']) ? $metadata['mediamosa-asset-type'] : NULL
    );

    // Update all metadata.
    $metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full(FALSE, array('dublin_core', 'qualified_dublin_core'));
    if (isset($metadata)) {
      mediamosa_asset_metadata::metadata_create($asset_id, $metadata_definitions_full, $metadata, 'replace');
    }

    // Collections.
    if (isset($metadata['mediamosa-collection-id'][0])) {
      foreach ($metadata['mediamosa-collection-id'] as $coll_id) {
        // Test if collection exists.
        $coll = mediamosa_collection::search(array(), array('coll_id' => $coll_id, 'app_id' => $app_id));
        $coll = reset($coll);
        // Add the asset to the collection (if not already so).
        if (isset($coll) && !(empty($coll))) {
          mediamosa_asset_collection::create($app_id, (string) $owner, TRUE, $asset_id, $coll['coll_id']);
        }
      }
    }
    if (isset($metadata['mediamosa-collection'][0])) {
      foreach ($metadata['mediamosa-collection'] as $coll_title) {
        // Test if collection exists.
        $coll = mediamosa_collection::search(array(), array('title' => $coll_title, 'app_id' => $app_id));
        $coll = reset($coll);
        // Add the asset to the collection (if not already so).
        if (isset($coll) && !(empty($coll))) {
          mediamosa_asset_collection::create($app_id, $owner, TRUE, $asset_id, $coll['coll_id']);
        }
      }
    }

    // Transcode profiles on this server:
    $transcode_profiles = _drush_mediamosa_transcode_profiles();

    // Copy files.
    foreach ($bag->getBagContents() as $filepath) {
      $bag_filename = str_replace($bag->bagDirectory . '/data/', '', $filepath);

      if ($update_asset) {
        // If we are updating the asset, the bag_filename == mediafile_id, first
        // remove the existing mediafile id.
        $mediafile = mediamosa_asset_mediafile::get($bag_filename);
        if (!empty($mediafile)) {
          mediamosa_asset_mediafile::delete($mediafile);
        }
      }

      // Determine flags from bag-info.txt.
      $source_id = NULL;
      $is_original = 'TRUE';
      $profile_id = NULL;
      $filename = '';
      // Alternative filename given?
      if (isset($metadata['mediamosa_filename_' . strtolower($bag_filename)][0])) {
        $filename = $metadata['mediamosa_filename_' . strtolower($bag_filename)][0];
      }
      else {
        $filename = $bag_filename;
      }
      // Transcode data (profile_id).
      if (isset($metadata['mediamosa_profile_id_' . strtolower($bag_filename)][0])) {
        $is_original = 'FALSE';
        // @todo.
        $source_id = 'FROM_IMPORT';
        $profile_id = $metadata['mediamosa_profile_id_' . strtolower($bag_filename)][0];
        if (isset($metadata['mediamosa_profile_' . strtolower($bag_filename)][0])) {
          // We have a profile name: get the id from this server.
          $profile = $metadata['mediamosa_profile_' . strtolower($bag_filename)][0];
          $profile_id_search = array_search($profile, $transcode_profiles);
          $profile_id = ($profile_id_search == FALSE) ? $profile_id : $profile_id_search;
        }
      }
      $is_still = 'FALSE';
      if (isset($metadata['mediamosa_is_still'])) {
        if (array_search($bag_filename, $metadata['mediamosa_is_still']) !== FALSE) {
          $is_still = 'TRUE';
          $is_original = 'FALSE';
        }
      }
      $downloadable = 'FALSE';
      if (isset($metadata['mediamosa_downloadable'])) {
        if (array_search($bag_filename, $metadata['mediamosa_downloadable']) !== FALSE) {
          $downloadable = 'TRUE';
        }
      }

      // Add mediafile id and store relavative pathname in metadata.
      $mediafile_id = mediamosa_asset_mediafile::create(
        mediamosa_db::uuid($app_id),
        $app_id,
        $asset_id,
        $owner,
        array(
          mediamosa_asset_mediafile_db::FILENAME => $filename,
          mediamosa_asset_mediafile_db::IS_STILL => $is_still,
          mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE => $source_id,
          mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE => $is_original,
          mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID => $profile_id,
          mediamosa_asset_mediafile_db::IS_DOWNLOADABLE => $downloadable,
        ));

      // filecopy.
      if (method_exists('mediamosa_io', 'realpath')) {
        $mm_file_path = mediamosa_io::realpath(mediamosa_storage::create_storage_uri($app_id, $mediafile_id));
      }
      else {
        $mm_file_path = mediamosa_configuration_storage::mediafile_id_filename_get($mediafile_id);
      }
      $res = mediamosa_io::copy($filepath, $mm_file_path);
      if (!$res) {
        return drush_set_error('', dt('Filecopy failed, did you run this as a webserver user?'));
      }
      // Start analyse.
      drush_mediamosa_mm_analyse($mediafile_id);

      if (isset($metadata['mediamosa-generate-still'])) {
        if (($is_original == 'TRUE') and ($is_still == 'FALSE')) {
          if (array_search($bag_filename, $metadata['mediamosa-generate-still']) !== FALSE) {
            drush_mediamosa_mm_still($mediafile_id);
          }
        }
      }
      if (isset($metadata['mediamosa-transcode'])) {
        if (array_search($bag_filename, $metadata['mediamosa-transcode']) !== FALSE) {
          if (isset($metadata['mediamosa-transcode-profile'])) {
            drush_mediamosa_mm_transcode($mediafile_id, $metadata['mediamosa-transcode-profile']);
          }
        }
      }

      // ACL.
      if ((isset($metadata['mediamosa-acl'])) || (isset($metadata['mediamosa-share']))) {
        drush_mediamosa_mm_acl_set($mediafile_id, $metadata['mediamosa-acl'][0], $metadata['mediamosa-share'][0]);
      }
      echo '.';
    }
    drush_print('done; asset id : ' . $asset_id);
  }
  else {
    drush_print('Errors found in validating bag.');
    // drush_print_table($bag->getBagErrors());
  }
}

/**
 * Helper function to get the transcode profiles.
 */
function _drush_mediamosa_transcode_profiles() {
  $rows = drupal_static('_drush_mediamosa_transcode_profiles');
  if (!isset($rows)) {
    $transcode_profiles = mediamosa_transcode_profile::search('');
    $rows = array();
    foreach ($transcode_profiles as $transcode_profile) {
      $rows[$transcode_profile['transcode_profile_id']] = $transcode_profile['profile'];
    }
  }
  return $rows;
}

/**
 * Callback function for mm-export-bagit command.
 *
 * Export asset to a Bagit.
 */
function drush_mediamosa_mm_export_bagit($asset_id, $path = NULL) {
  $transcode_profiles = _drush_mediamosa_transcode_profiles();

  // Sanitize path.
  if ($path == NULL) {
    $path = getcwd() . '/';
  }
  else {
    $path .= (drupal_substr($path, -1) == '/' ? '' : '/');
  }

  $quiet = drush_get_option('quiet', NULL);

  // Get asset.
  if ($asset_id == NULL) {
    $asset_id = 0;
  }
  $asset = mediamosa_asset::get($asset_id, NULL);
  if (!$asset) {
    return drush_set_error('', dt('Invalid asset_id.'));
  }

  // Add asset id to path.
  $path .= $asset_id . '/';

  // Start BagIt.
  $baginfo = array();

  // Basic asset information.
  $asset_info_fields = array(
    'Mediamosa-asset-id' => 'asset_id',
    'MediaMosa-reference-id' => 'reference_id',
    'MediaMosa-provider-id' => 'provider_id',
    'MediaMosa-owner-id' => 'owner_id',
    'MediaMosa-group-id' => 'group_id',
    'MediaMosa-played' => 'played',
    'MediaMosa-viewed' => 'viewed',
    'MediaMosa-isprivate' => 'isprivate',
    'MediaMosa-play-restriction-start' => 'play_restriction_start',
    'MediaMosa-play-restriction-end' => 'play_restriction_end',
    'MediaMosa-is-unappropriate' => 'is_unappropriate',
    'MediaMosa-video-type' => 'video_type',
  );
  foreach ($asset_info_fields as $bagit_prop => $prop) {
    if (isset($asset[$prop])) {
      $baginfo[$bagit_prop] = $asset[$prop];
    }
  }

  // Get metadata.
  $metadata = mediamosa_asset_metadata::metadata_get($asset_id);
  foreach ($metadata as $metadata_name => $metadata_entry) {
    foreach ($metadata_entry as $name => $value) {
      foreach ($value['values'] as $val_entry) {
        $baginfo[_mediamosa_ck_bagit_metadatagroup_convert($metadata_name, FALSE) . '-' . $name] = $val_entry;
      }
    }
  }

  $bagitlib = libraries_detect('bagitphp');
  if ($bagitlib && !empty($bagitlib['installed'])) {
    libraries_load('bagitphp');
  }
  else {
    drupal_set_message('No bagit library detected.', 'error');
    return;
  }

  $bag = new BagIt($path, TRUE, TRUE, TRUE, $baginfo);

  // Add the files.
  $fields = array(
    mediamosa_asset_mediafile_db::ID,
    mediamosa_asset_mediafile_db::CREATED,
    mediamosa_asset_mediafile_db::TRANSCODE_PROFILE_ID,
    mediamosa_asset_mediafile_db::FILENAME,
    mediamosa_asset_mediafile_db::IS_STILL,
    mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE,
    mediamosa_asset_mediafile_db::MEDIAFILE_ID_SOURCE,
    mediamosa_asset_mediafile_db::IS_DOWNLOADABLE,
  );

  $mediafiles = mediamosa_asset_mediafile::get_by_asset_id($asset_id, $fields, array('orderby' => mediamosa_asset_mediafile_db::CREATED));
  foreach ($mediafiles as $mediafile) {
    $p = mediamosa_io::realpath(mediamosa_storage::get_uri_mediafile($mediafile['mediafile_id']));
    if (file_exists($p)) {
      $bag->addFile($p, $mediafile['mediafile_id']);

      // Store extra info about the files in bag-info.txt.
      $bag->setBagInfoData('mediamosa_filename_' . $mediafile['mediafile_id'], $mediafile['filename']);
      if ($mediafile[mediamosa_asset_mediafile_db::IS_DOWNLOADABLE] == 'TRUE') {
        $bag->setBagInfoData('mediamosa_downloadable', $mediafile['mediafile_id']);
      }
      if ($mediafile[mediamosa_asset_mediafile_db::IS_STILL] == 'TRUE') {
        $bag->setBagInfoData('mediamosa_is_still', $mediafile['mediafile_id']);
        $bag->setBagInfoData('mediamosa_still_source_' . $mediafile['mediafile_id'], $mediafile['mediafile_id_source']);
      }
      elseif ($mediafile[mediamosa_asset_mediafile_db::IS_ORIGINAL_FILE] == 'FALSE') {
        $bag->setBagInfoData('mediamosa_profile_id_' . $mediafile['mediafile_id'], $mediafile['transcode_profile_id']);
        $bag->setBagInfoData('mediamosa_profile_' . $mediafile['mediafile_id'], $transcode_profiles[$mediafile['transcode_profile_id']]);
        $bag->setBagInfoData('mediamosa_profile_source_' . $mediafile['mediafile_id'], $mediafile['mediafile_id_source']);
      }
      else {
        $bag->setBagInfoData('mediamosa_is_original', $mediafile['mediafile_id']);
      }
      // @todo: acl.
    }
  }
  $bag->update();
}

/**
 * Callback function for mm-export-bagit-app command.
 *
 * Export all assets to Bagits.
 */
function drush_mediamosa_mm_export_bagit_app($app_id, $path = NULL) {

  $nr_assets_per_loop = 100;

  $cql = drush_get_option('cql', '');

  $search_options = array(
    'app_ids' => array($app_id),
    'cql' => $cql,
    'cql_store_stats' => TRUE,
    'granted' => TRUE,
    'is_public_list' => FALSE,
    'is_app_admin' => TRUE,
    'hide_empty_assets' => 'TRUE',
    'calculate_total_count' => TRUE,
    'show_deleted' => FALSE,
    'limit' => $nr_assets_per_loop,
    'offset' => 0,
  );

  $assets = mediamosa_search::asset($search_options);
  drush_print('Total of assets to export: ' . $assets['total_count']);

  for ($i = 0; $i < $assets['total_count']; $i += $nr_assets_per_loop) {
    foreach ($assets['asset_ids'] as $asset_id) {
      echo "\n" . $asset_id . " ";
      drush_mediamosa_mm_export_bagit($asset_id, $path);
    }
    $search_options['offset'] += $nr_assets_per_loop;
    $assets = mediamosa_search::asset($search_options);
  }
  echo "\n";
  drush_print('Assets exported to : ' . $path);
}

/**
 * Callback function for mm-import-bagit-dir command.
 *
 * Import a directory of Bagits in an app.
 */
function drush_mediamosa_mm_import_bagit_dir($app_id = NULL, $path = NULL, $owner = 'N.N.') {

  if ($path == NULL) {
    return drush_set_error('', dt('Invalid path.'));
  }
  // Sanitize path.
  $path .= (drupal_substr($path, -1) == '/' ? '' : '/');
  if ($app_id == NULL) {
    return drush_set_error('', dt('App id is required.'));
  }
  $quiet = drush_get_option('quiet', NULL);
  $maintain_ids = drush_get_option('maintain_ids', FALSE);

  $num_of_dirs = count(scandir($path)) - 2;
  drush_print('About to import ' . $num_of_dirs . ' bagits.');
  if ($num_of_dirs == 0) {
    if (!$quiet) {
      drush_print('No files found in directory.');
    }
    else {
      return FALSE;
    }
  }
  else {
    // Loop directory recursively, to add files to the asset.
    $objects = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($path), RecursiveIteratorIterator::SELF_FIRST);
    foreach (scandir($path) as $dir) {
      if (($dir == '.') or ($dir == '..')) {
        continue;
      }
      echo $dir . "; ";
      drush_mediamosa_mm_import_bagit($path . $dir, $app_id, $owner);
    }
  }
}
