<?php
/**
* Implements hook_islandora_iiif_collection_query_backends().
*/
function islandora_iiif_islandora_iiif_collection_query_backends() {
  $module_path = drupal_get_path('module', 'islandora_solr');
  return array(
    'islandora_solr_query_backend' => array(
      'title' => t('Collection via Solr'),
      'callable' => 'islandora_iiif_islandora_collection_solr_backend_callable',
      'file' => "$module_path/includes/collection.inc",
    ),
  );
}
/**
* Implements callback_islandora_iiif_collection_query_backends()
*/
function islandora_iiif_islandora_collection_solr_backend_callable($object, $index) {
  module_load_include('inc', 'islandora_solr', 'includes/db');
  $qp = new IslandoraSolrQueryProcessor();

  $qp->buildQuery(format_string('!member_field:("info:fedora/!pid" OR "!pid") OR !collection_member_field:("info:fedora/!pid" OR "!pid")', array(
    '!member_field' => variable_get('islandora_solr_member_of_field', 'RELS_EXT_isMemberOf_uri_ms'),
    '!pid' => $object->id,
    '!collection_member_field' => variable_get('islandora_solr_member_of_collection_field', 'RELS_EXT_isMemberOfCollection_uri_ms'),
  )), drupal_get_query_parameters());

  $limit = variable_get('islandora_solr_num_of_results', '20');

  $qp->solrStart = $index;
  $qp->solrLimit = $limit;

  $qp->solrParams['sort'] = $qp->internalSolrParams['sort'];

  $qp->executeQuery();

  dpm($qp->islandoraSolrResult['response']);

  return array(
    $qp->islandoraSolrResult['response']['numFound'],
    $qp->islandoraSolrResult['response']['objects'],
  );
};
  
/**
* This is the actual collection building function
*/ 
function islandora_iiif_collection_callback(AbstractObject $object, $index = 1) {
  module_load_include('inc', 'islandora_solr_metadata', 'includes/db');
  module_load_include('inc', 'islandora_solr_metadata', 'theme/theme');
  module_load_include('inc', 'islandora_iiif', 'includes/sequence');
 
  if (!in_array('islandora:collectionCModel', $object->models)) {
    drupal_not_found();
  }
 
  $limit = variable_get('islandora_solr_num_of_results', '20');
  
  $children_list = islandora_iiif_islandora_collection_solr_backend_callable($object, $index);
  $total_children = $children[0];
  $current_response = $children[1];
  $collection_members[] = array();
  $istop = variable_get('islandora_repository_pid', 'islandora:root') == $object->id;
  if ($children_list[0] != 0) {

  foreach ($children_list [1] as $children_info) {
    $label = "";
    $id = urldecode(url("islandora_iiif/{$canvas_info['PID']}/manifest", array(
      'absolute' => TRUE,
      'language' => (object)array('language' => FALSE),
      )));
    $label = $children_info['object_label'];  
        $collection_members[] = array(
          "@id" => $id,
          "@type" => "sc:Manifest",
          "label" => $label,
        );
 
  }
  } 
  
  $collection = new \stdClass;
  $collection->{'@context'} = "http://iiif.io/api/presentation/2/context.json";
  $collection->{'@id'} = urldecode(url(current_path(),
     array(
      'absolute' => TRUE,
      'language' => (object)array('language' => FALSE),
    ))
  );
  $collection->{'@type'} = "sc:Collection";
  
  $collection->label = $object->label;
  $next_url_str = NULL;
  $prev_url_str = NULL;
  $current_breadcrumb = drupal_get_breadcrumb();
  dpm($current_breadcrumb);
  
  // Few assumptiomns
  // A. If total results is less than $limit, then no $next,
  // B. if $index + $limit is more than total, then no $next
  // C. if $index - $limit <= 0 then $prev is parent top
  // if $index = 1 then no $prev

  if (($total_children > $index+$limit) && ($total_children>0)) {
    $next_url_str = "islandora_iiif/collection/{$object->id}/{($index+$limit+1)}";
  }
   if (($index > $limit) && ($total_children>0)) {
     $prev_url_str = "islandora_iiif/collection/{$object->id}/{($index-$limit+1}";
  }
  
  if ($next_url_str) {
  $collection->next = array(
    "@id" => urldecode(url($next_url_str, array(
      'absolute' => TRUE,
      'language' => (object)array('language' => FALSE),
    )))
  );
  }
  if ($prev_url_str) { 
    $collection->prev = array(
      "@id" => urldecode(url($prev_url_str, array(
        'absolute' => TRUE,
        'language' => (object)array('language' => FALSE),
      )))
    );
  }
  
  $collection->within = array(
    "@id" => urldecode(url("islandora_iiif/collection/{$object->id}", array(
      'absolute' => TRUE,
      'language' => (object)array('language' => FALSE),
    ))
      )
  );


 // And to make this legit we need a sequence with some canvases

drupal_add_http_header('Content-Type', 'application/json; charset=utf-8');
echo json_encode($collection, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE);
drupal_page_footer();
 
}