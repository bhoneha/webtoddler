#include "curl/curl.h"
#include "lexbor/html/parser.h"
#include "lexbor/dom/interfaces/element.h"
#include <iostream>
#include <string>
#include <queue>
#include <unordered_set>

#define MAX_DEPTH 1
#define MAX_PAGES 1

void getLeafNode(lxb_dom_node_t*&);
std::string downloadHTML(CURL* curl, std::string URL, std::unordered_set<std::string>& visited_links);
void extractLink(
	std::queue<std::pair<std::string, int>>& q,
	const lxb_char_t* html_content,
	size_t size,
	std::unordered_set <std::string>& visited_links,
	int depth);
std::string relativeToAbsoluteURL(std::string domain, std::string url);

// callback function to store downloaded HTML into std::string
size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
	size_t total_size = size * nmemb;
	std::string* out = static_cast<std::string*>(userdata);
	out->append(ptr, total_size);
	return total_size;
}


int main() {

	std::queue <std::pair<std::string,int>> q; // URL Queues
	std::unordered_set<std::string> visited_links;
	// root URL
	std::string seed = "http://localhost:80";

	const lxb_char_t* html_content;
	std::string content = "";
	std::string currentURL = "";
	int depth = 0;
	int pages = 0;

	// libcurl setup
	CURL* curl = curl_easy_init();

	q.push({ seed,0 });


	while (!q.empty()) {
		if (pages >= MAX_PAGES) break;
		pages++;
		currentURL = relativeToAbsoluteURL(seed, q.front().first);
		
		depth = q.front().second;
		content = downloadHTML(curl, currentURL, visited_links);
		if (!content.empty()) {
			html_content = (const lxb_char_t*)content.c_str();
			extractLink(q, html_content, content.size(), visited_links,depth);
		}
		q.pop();
	}
	curl_easy_cleanup(curl);
	
	std::cout << "----- Visited Links ------" << '\n';
	for (std::string link : visited_links) {
		std::cout << link << '\n';
	}

}


std::string downloadHTML(CURL* curl, std::string URL, std::unordered_set<std::string>& visited_links) {

	std::string html_content = "";

	// setting curl option
	curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html_content);

	if (visited_links.find(URL) == visited_links.end()) {
		// download the html content
		CURLcode res = curl_easy_perform(curl);
		if (res == CURLE_OK) {
			std::cout << "downloaded successfully" << '\n';
			visited_links.insert(URL);
		}
		else {
			std::cout << res << '\n';
		}
	}
	

	return html_content;
}


void extractLink(
	std::queue<std::pair<std::string,int>>& q, 
	const lxb_char_t* html_content, 
	size_t size,
	std::unordered_set <std::string>& visited_links,
	int depth) {

	if (depth > MAX_DEPTH) return;

	std::string link;
	lxb_html_document_t* html_document = lxb_html_document_create();
	lxb_status_t status;

	status = lxb_html_document_parse(html_document, html_content, size);
	if (status != LXB_STATUS_OK) {
		exit(EXIT_FAILURE);
	}

	// DFS DOM Traversal
	const lxb_char_t* attr = (const lxb_char_t*)"href";
	lxb_dom_attr* href = NULL;
	lxb_dom_node_t* target = NULL;

	// get the body as root node
	lxb_dom_node_t* root = lxb_dom_interface_node(html_document->body);

	target = root;

	// if the root has no children, exit
	if (root->first_child == NULL) {
		std::cout << "Root has no children" << std::endl;
		exit(EXIT_SUCCESS);
	}

	// check if it reaches the leaf node
	getLeafNode(target);
	while (target != root) {
		lxb_dom_element_t* target_element = lxb_dom_interface_element(target);


		if (!strcmp((const char*)lxb_dom_element_qualified_name(target_element, NULL), "a") && target->type == LXB_DOM_NODE_TYPE_ELEMENT) {
			lxb_dom_attr* href = lxb_dom_element_attr_by_name(target_element, (const lxb_char_t*)"href", 4);
			if (href == NULL) { std::cout << "Attribute is null" << std::endl; }
			link = (const char*)lxb_dom_attr_value(href, NULL);
			if (visited_links.find(link) == visited_links.end()) {
				q.push({ link, depth + 1});
				std::cout << link << std::endl;
			}
			else {
				std::cout << "Found already : " << link << std::endl;
			}

		}


		if (target->next != NULL) {
			target = target->next;
			getLeafNode(target);
		}
		else {
			target = target->parent;

		}
	}

	lxb_html_document_destroy(html_document);

}



void getLeafNode(lxb_dom_node_t*& target) {
	while (target->first_child != NULL) {
		target = target->first_child;
		lxb_dom_element_t* element = lxb_dom_interface_element(target);
	}
}


std::string relativeToAbsoluteURL(std::string domain, std::string url) {
	if (url.find("https", 0) == std::string::npos) {
		if (url.find("http", 0) == std::string::npos) {
			if(url[0] != '/') domain += "/";
			return domain + url;
		}
	}

	return url;
}
