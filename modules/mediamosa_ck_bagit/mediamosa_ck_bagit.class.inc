<?php
/**
 * @file
 * The bagit class.
 */

class mediamosa_ck_bagit {

  // ---------------------------------------------------------------- Functions.
  /**
   * Store the metadata contents into the asset.
   *
   * @param array $bag_info_data
   *   The array data parsed from the bag_info.txt.
   * @param string $action.
   *   Either "replace" or "append" or "update". See asset/$asset_id/metadata
   *   REST call documentation.
   *
   */
  public static function commit_bag_info_data($asset_id, $bag_info_data, $action = 'update') {
    // Get our metadata definition.
    $metadata_fields = mediamosa_ck::get_metadata_fields();

    $metadata = array();
    foreach ($bag_info_data as $name => $fields) {
      if (strpos($name, '-') === FALSE) {
        continue;
      }

      // Split the name.
      list($set, $metadata_name) = explode('-', $name, 2);
      $set = trim(strtolower($set));
      $metadata_name = trim(strtolower($metadata_name));

      if (isset($metadata_fields[$set]) && isset($metadata_fields[$set]['fields']) && isset($metadata_fields[$set]['fields'][$metadata_name])) {
        $values = self::parse_info_data($fields);

        if (count($values)) {
          $metadata[$metadata_name] = $values;
        }
      }
    }

    // Do the REST call.
    if (!empty($metadata)) {
      unset($metadata['user_id']);
      $metadata['action'] = $action;
      if (MediaMosaCkConnectorWrapper::update_metadata($asset_id, $metadata)) {
        drupal_set_message('Metadata updated');
        return TRUE;
      }

      drupal_set_message('REST call failed to update the metadata.');
      return FALSE;
    }

    drupal_set_message('No metadata was found to set');
    return FALSE;
  }

  /**
   * Helper function for commit_bag_info_data().
   *
   * @param array|string $fields
   *   The fields to parse.
   *
   * @return array
   *   The parsed data.
   */
  protected static function parse_info_data($fields) {
    $metadata = array();
    if (is_array($fields)) {
      foreach ($fields as $field) {
        $data = self::parse_info_data($field);
        if ($data !== FALSE) {
          $metadata = array_merge($metadata, $data);
        }
      }
    }
    else {
      $data = trim((string) $fields);
      return $data !== '' ? array($data) : FALSE;
    }

    return $metadata;
  }
}