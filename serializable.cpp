#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sstream>
#include <vector>
#include <iomanip>
#include <iostream>
#include <string>
#include <algorithm>

#include <rapidjson/prettywriter.h>

#include <ether.h>

#include <serializable/serializable.h>
#include <serializable/utils.h>

#include <common/output_format_file.h>

#include <odc/manager.h>

#include <odc/internals/transaction/file.h>

#include <utils/string.h>

#define serialize_array_not_empty(w, c) \
    do { \
        w.StartArray(); \
        for (auto &&i: c) { \
            if (not i.empty()) \
                serializable::serialize(w, i); \
        } \
        w.EndArray(); \
    } while (0)

#define serialize_array(w, c) \
    do { \
        w.StartArray(); \
        for (auto &&i: c) { \
            serializable::serialize(w, i); \
        } \
        w.EndArray(); \
    } while (0)

#define serialize_array_if_not(w, c, v) \
    do { \
        w.StartArray(); \
        for (auto &&i: c) { \
            if (i != v) \
                serializable::serialize(w, i); \
        } \
        w.EndArray(); \
    } while (0)


#define SERIAL_ARRAY(field) \
    if (field.size()){ \
        w.String(#field); \
        serialize_array(w, field); }

#define cef_row(t, c, v, p) \
    do { \
		if (!v.empty()) { \
			dest << p + #t "." #c "=" << OutputFormatFile::NoSep() << v; \
		} \
    } while (0)

namespace serializable {
void
Email::serialize(Writer &w) const
{
    ////[EMAIL]
   ////euser|string|email user name 
   w.String("euser");             w.String(user.c_str());
   ////euser|string|email user name 
   w.String("epassword");         w.String(password.c_str());

   ////emails|array|info about mailcontent
   w.String("emails");
   w.StartArray();
   for (auto &&m: emails) {
       w.StartObject();
       //EMAIL FROM
       ////emails.esrc|object| there is 2 differents source format
       w.String("esrc");
       w.StartObject();
       ////emails.esrc.f|string|fullname
       w.String("f");   w.String(m.from.first.c_str());
       ////emails.esrc.o|string|original
       w.String("o");   w.String(m.from.second.c_str());
       w.EndObject();
       //Envelope
       
       ////emails.envelopesrc|string|original
       if (!m.envelope_from.empty()) { w.String("envelopesrc"); w.String(m.envelope_from.c_str()); }

       //EMAIL TO
       if (!m.to.empty()) {
           w.String("edst");
           w.StartArray();
           for (auto &&j: m.to) {
			   w.StartObject();
			   w.String("f");  w.String(j.first.c_str());
			   w.String("o");  w.String(j.second.c_str());
			   w.String("d");  w.String(j.second.c_str());
			   w.EndObject();
           }
           w.EndArray();
       }
       //Envelope
       if (!m.envelope_to.empty()) {
           w.String("envelopedst");
           w.StartArray();
           for (auto &&j: m.envelope_to) {
               w.String(j.c_str());
           }
           w.EndArray();
       }

       //EMAIL CC
       if (!m.cc.empty()) {
           w.String("ecc");
           w.StartArray();
           for (auto &&j: m.cc) {
			   w.StartObject();
			   w.String("f");  w.String(j.first.c_str());
			   w.String("o");  w.String(j.second.c_str());
			   w.EndObject();
           }
           w.EndArray();
       }

       //EMAIL BCC
       if (!m.bcc.empty()) {
           w.String("ebcc");
           w.StartArray();
           for (auto &&j: m.bcc) {
			   w.StartObject();
			   w.String("f");  w.String(j.first.c_str());
			   w.String("o");  w.String(j.second.c_str());
			   w.EndObject();
           }
           w.EndArray();
       }

       //Date
       if (!m.sent_date.empty()) { w.String("sdate");   w.String(m.sent_date.c_str()); }

       //EMAIL SUBJECT
       if (!m.subject.empty()) { w.String("esub");       w.String(m.subject.c_str()); }

       if (!m.message_id.empty()) {  w.String("emessage_id");           w.String(m.message_id.c_str()); }
       if (!m.in_reply_to.empty()) {  w.String("ein_reply_to");         w.String(m.in_reply_to.c_str()); }
       if (!m.references.empty()) {  w.String("ereferences");           w.String(m.references.c_str()); }
       if (!m.comments.empty()) {  w.String("ecomments");               w.String(m.comments.c_str()); }
       if (!m.keywords.empty()) {  w.String("ekeywords");               w.String(m.keywords.c_str()); }
       if (!m.trace.empty()) {  w.String("etrace");                     w.String(m.trace.c_str()); }
       if (!m.resent_date.empty()) {  w.String("eresent_date");         w.String(m.resent_date.c_str()); }
       // if (!m.resent_from.empty()) {  w.String("eresent_from");         w.String(m.resent_from.c_str()); }
       //if (!m.resent_sender.empty()) {  w.String("eresent_sender");     w.String(m.resent_sender.c_str()); }
       w.String("dir");
       switch (m.direction) {
        case e_direction::FETCH:
            w.String("r");  //Receive
            break;
        case e_direction::SEND:
            w.String("s");  //Sent
            break;
        case e_direction::STORE:
            w.String("d");  //Draft
            break;
        default:
            w.String("u");  //Unknown
            break;
       }

	   if (!m.bodies.empty()) {
		   w.String("ebod");

		   w.StartArray();
		   for (auto &&j: m.bodies) {
			   w.StartObject();
			   w.String("t");  w.String(j.content_type.c_str());
			   //w.String("e");  w.String(j.transfer_encoding.c_str());
			   w.String("b");  w.String(j.body.c_str());
			   w.EndObject();
		   }
           w.EndArray();
	   }

       //if (!m.attachments.empty()) {
       //    w.String("efn");        serialize_array_not_empty(w, m.attachments);
       //}

	   if (m.attachment_files.size() != 0) {
		   w.String("ef");
		   w.StartArray();
		   auto mapfiles = m.attachment_files;
		   for (auto it = mapfiles.begin(); it != mapfiles.end(); ++it ) {
			   it->second->get_serializable_file().serialize(w);
		   }
		   w.EndArray();
	   }

       w.EndObject();
   }
   w.EndArray();
}

const std::string Email::csv_prefix = "email";
const std::vector<std::string> Email::csv_fields = {
	"from", "to", "cc", "bcc", "message_id", "subject", "attachements", "user", "password"
};

void
Email::to_csv(OutputFormatFile &dest) const
{
/*   dest << emails[0].from,first
      << emails[0].to
      << emails[0].cc
      << emails[0].bcc
	  << emails[0].message_id
      << emails[0].subject
      << emails[0].attachments
      << user
      << password;
	  */
}

void
Email::to_cef(OutputFormatFile &dest, std::string prefix) const
{
	cef_row("email", "users" ,user, prefix);
	cef_row("email", "passwords", password, prefix);
	cef_row("email", "form", emails[0].from.first, prefix);
	cef_row("email", "to", emails[0].to, prefix);
	cef_row("email", "to", emails[0].cc, prefix);
	cef_row("email", "to", emails[0].bcc, prefix);
	cef_row("email", "to", emails[0].message_id, prefix);
	cef_row("email", "subjects", emails[0].subject, prefix);
	cef_row("email", "attachments", emails[0].attachments, prefix);
}

void
PS4Chat::serialize(Writer &w) const
{
    w.String("psn");
    w.StartObject();
    w.String("user1-name");    w.String(user1_name.c_str());
    w.String("user2-name");    w.String(user2_name.c_str());
    w.String("user1-country"); w.String(user1_country.c_str());
    w.String("user2-country"); w.String(user2_country.c_str());
    w.String("user1-console"); w.String(user1_console.c_str());
    w.String("user2-console"); w.String(user2_console.c_str());
    w.EndObject();
}

const std::string PS4Chat::csv_prefix = "psn";
const std::vector<std::string> PS4Chat::csv_fields = {
    "username_1", "username_2", "country_1", "country_2",
    "console_1", "console_2"
};

void
PS4Chat::to_csv(OutputFormatFile &dest) const
{
    dest << user1_name
        << user2_name
        << user1_country
        << user2_country
        << user1_console
        << user2_console;
}

void
PS4Chat::to_cef(OutputFormatFile &dest, std::string prefix) const
{
	cef_row("ps4", "user1_name", user1_name, prefix);
	cef_row("ps4", "user2_name", user2_name, prefix);
	cef_row("ps4", "user1_country", user1_country, prefix);
	cef_row("ps4", "user2_country", user2_country, prefix);
	cef_row("ps4", "user1_console", user1_console, prefix);
	cef_row("ps4", "user2_console", user2_console, prefix);
}

void
Irc::serialize(Writer &w) const
{
    w.String("irc");
    w.StartObject();
    if (nicks.size()) {
        w.String("u"); serialize_array(w, nicks);
    }
    if (passwords.size()) {
        w.String("p"); serialize_array(w, passwords);
    }
    if (channels.size()) {
        w.String("c"); serialize_array(w, channels);
    }
    w.EndObject();
}

const std::string Irc::csv_prefix = "irc";
const std::vector<std::string> Irc::csv_fields = {
     "username", "password", "channel"
};

void
Irc::to_csv(OutputFormatFile &dest) const
{
    dest << nicks << passwords << channels;
}

void Irc::to_cef(OutputFormatFile &dest, std::string prefix) const{
	cef_row("irc", "username", nicks, prefix);
	cef_row("irc", "passwords", passwords, prefix);
	cef_row("irc", "channels", channels, prefix);
}

void
Http::serialize(Writer &w) const
{
    w.String("http");
    w.StartObject();
    if (statuscode.size()) {
        w.String("statuscode");       serialize_array(w, statuscode);
    }
    if (ct_src.size()) {
        w.String("ct-src");           serialize_array(w, ct_src);
    }
    if (ct_dst.size()) {
        w.String("ct-dst");           serialize_array(w, ct_dst);
    }
    if (ct_dst.size() or ct_src.size()) {
        std::vector<std::string> tmp(ct_src.begin(), ct_src.end());
        tmp.insert(tmp.end(), ct_dst.begin(), ct_dst.end());
        w.String("bodymagic-term");   serialize_array(w, tmp);
    }
    if (method_term.size()) {
        w.String("method-term");      serialize_array_not_empty(w, method_term);
    }

    w.String("u");
    w.StartArray();
    for (auto& auth: auths) {
        serializable::serialize(w, auth.first.first);
    }
    w.EndArray();
    w.String("p");
    w.StartArray();
    for (auto& auth: auths) {
        serializable::serialize(w, auth.first.second);
    }
    w.EndArray();

    w.EndObject();

    if (hpath.size()) {
        w.String("hpath");            serialize_array(w, hpath);
    }
    if (uri_params.size()) {
        w.String("hkey");
        w.StartArray();
        for (auto const &it : uri_params) {
            w.String(it.first.c_str());
        }
        w.EndArray();
        w.String("hval");
        w.StartArray();
        for (auto const &it : uri_params) {
            w.String(it.second.c_str());
        }
        w.EndArray();
    }
    if (cookie_params.size()) {
        w.String("hckey-term");
        w.StartArray();
        for (auto const &it : cookie_params) {
            w.String(it.first.c_str());
        }
        w.EndArray();
        w.String("hcval-term");
        w.StartArray();
        for (auto const &it : cookie_params) {
            w.String(it.second.c_str());
        }
        w.EndArray();
    }
    if (ho.size()) {
        w.String("ho");               serialize_array_not_empty(w, ho);
    }
    if (ua.size()) {
        w.String("ua");               serialize_array_not_empty(w, ua);
    }
    if (ref.size()) {
        w.String("ref");              serialize_array_not_empty(w, ref);
    }
}

const std::string Http::csv_prefix = "http";
const std::vector<std::string> Http::csv_fields = {
    "host", "path", "method", "status_code", "referrer", "content-type_src",
    "content-type_dst", "auth", "user_agent", "query_key", "query_value",
    "cookie_key", "cookie_value"
};

void
Http::to_csv(OutputFormatFile & dest) const
{
    std::vector<std::pair<std::string, std::string>> auths_(auths.size());
    for (auto &&i: auths) {
        auths_[i.second] = i.first;
    }
    auto const get_first = [] (std::pair<std::string, std::string> const &it) -> std::string {
        return it.first;
    };
    auto const get_second = [] (std::pair<std::string, std::string> const &it) -> std::string {
        return it.second;
    };
    std::vector<std::string> hkey, hval, hckey_term, hcval_term;
    std::transform(uri_params.begin(), uri_params.end(), std::back_inserter(hkey),  get_first);
    std::transform(uri_params.begin(), uri_params.end(), std::back_inserter(hval),  get_second);
    std::transform(cookie_params.begin(), cookie_params.end(), std::back_inserter(hckey_term),  get_first);
    std::transform(cookie_params.begin(), cookie_params.end(), std::back_inserter(hcval_term),  get_second);

    dest << ho
         << hpath
         << method_term
         << statuscode
         << ref
         << ct_src
         << ct_dst
         << auths_
         << ua
         << hkey
         << hval
         << hckey_term
         << hcval_term;
}

void Http::to_cef(OutputFormatFile &dest, std::string prefix) const {

    std::vector<std::pair<std::string, std::string>> auths_(auths.size());
    for (auto &&i: auths) {
        auths_[i.second] = i.first;
    }
    auto const get_first = [] (std::pair<std::string, std::string> const &it) -> std::string {
        return it.first;
    };
    auto const get_second = [] (std::pair<std::string, std::string> const &it) -> std::string {
        return it.second;
    };
    std::vector<std::string> hkey, hval, hckey_term, hcval_term;
    std::transform(uri_params.begin(), uri_params.end(), std::back_inserter(hkey),  get_first);
    std::transform(uri_params.begin(), uri_params.end(), std::back_inserter(hval),  get_second);
    std::transform(cookie_params.begin(), cookie_params.end(), std::back_inserter(hckey_term),  get_first);
    std::transform(cookie_params.begin(), cookie_params.end(), std::back_inserter(hcval_term),  get_second);


	cef_row("http", "host", ho, prefix);
	cef_row("http", "path", hpath, prefix);

	cef_row("http", "method", method_term, prefix);
	cef_row("http", "status_code", statuscode, prefix);
	cef_row("http", "referrer", ref, prefix);
	cef_row("http", "content-type_src", ct_src, prefix);
	cef_row("http", "content-type_dst", ct_dst, prefix);


	cef_row("http", "auth_", auths_, prefix);
	//ua may contain one null elm
	cef_row("http", "user_agent", ua, prefix);
	cef_row("http", "query_key", hkey, prefix);

	cef_row("http", "query_valuel", hval, prefix);
	cef_row("http", "cookie_key", hckey_term, prefix);
	cef_row("http", "cookie_term", hcval_term, prefix);

}

void
Chat::serialize(Writer &w) const
{
    w.String("chat");

    w.StartObject();
    SERIAL_ARRAY(user_name);
    SERIAL_ARRAY(user_nickname);
    SERIAL_ARRAY(chatroom_open_name);
    SERIAL_ARRAY(chatroom_open_description);
    SERIAL_ARRAY(chatroom_close_name);
    SERIAL_ARRAY(chatroom_close_description);
    SERIAL_ARRAY(message);
    SERIAL_ARRAY(transfer_announ_file_name_array);
    SERIAL_ARRAY(transfer_file_name);
    SERIAL_ARRAY(transfer_file_description);
    SERIAL_ARRAY(chat_user_info);
    w.EndObject();
}

void
Chat::to_cef(OutputFormatFile &dest, std::string prefix) const {
	cef_row("chat", "user_name", user_name, prefix);
	cef_row("chat", "user_nickname", user_nickname, prefix);
    cef_row("chat", "chatroom_open_name", chatroom_open_name, prefix);
    cef_row("chat", "chatroom_open_description", chatroom_open_description, prefix);
    cef_row("chat", "chatroom_close_name", chatroom_close_name, prefix);
    cef_row("chat", "chatroom_close_description", chatroom_close_description, prefix);
    cef_row("chat", "message", message, prefix);
    cef_row("chat", "transfer_announ_file_name_array", transfer_announ_file_name_array, prefix);
    cef_row("chat", "transfer_file_name", transfer_file_name, prefix);
    cef_row("chat", "transfer_file_description", transfer_file_description, prefix);
    cef_row("chat", "chat_user_info", chat_user_info, prefix);
}

const std::string Chat::csv_prefix = "chat";
const std::vector<std::string> Chat::csv_fields = {
    "user_name",
    "user_nickname",
    "chatroom_open_name",
    "chatroom_open_description",
    "chatroom_close_name",
    "chatroom_close_description",
    "message",
    "transfer_announ_file_name_array",
    "transfer_file_name",
    "transfer_file_description",
    "chat_user_info"
};

void
Chat::to_csv(OutputFormatFile &dest) const
{
dest << user_name
    << user_nickname
    << chatroom_open_name
    << chatroom_open_description
    << chatroom_close_name
    << chatroom_close_description
    << message
    << transfer_announ_file_name_array
    << transfer_file_name
    << transfer_file_description
    << chat_user_info;

}

void
CSsl::serialize(Writer &w) const
{
    w.String("cssl");

    w.StartObject();
    SERIAL_ARRAY(signature);
    SERIAL_ARRAY(public_key);
    SERIAL_ARRAY(validity_not_before);
    SERIAL_ARRAY(validity_not_after);
    SERIAL_ARRAY(subject_common_name);
    SERIAL_ARRAY(subject_organizational_unit_name);
    SERIAL_ARRAY(subject_organization_name);
    SERIAL_ARRAY(subject_country_name);
    SERIAL_ARRAY(issuer_common_name);
    SERIAL_ARRAY(issuer_organizational_unit_name);
    SERIAL_ARRAY(issuer_organization_name);
    SERIAL_ARRAY(issuer_country_name);
    SERIAL_ARRAY(raw);
    SERIAL_ARRAY(issuer);
    SERIAL_ARRAY(subject);
    SERIAL_ARRAY(subject_locality_name);
    SERIAL_ARRAY(subject_state_or_province_name);
    SERIAL_ARRAY(issuer_locality_name);
    SERIAL_ARRAY(issuer_state_or_province_name);
    w.EndObject();
}

const std::string CSsl::csv_prefix = "cssl";
const std::vector<std::string> CSsl::csv_fields = {
    "signature",
    "public_key",
    "validity_not_before",
    "validity_not_after",
    "subject_common_name",
    "subject_organizational_unit_name",
    "subject_organization_name",
    "subject_country_name",
    "issuer_common_name",
    "issuer_organizational_unit_name",
    "issuer_organization_name",
    "issuer_country_name",
    "raw",
    "issuer",
    "subject",
    "subject_locality_name",
    "subject_state_or_province_name",
    "issuer_locality_name",
    "issuer_state_or_province_name"
};

void
CSsl::to_csv(OutputFormatFile &dest) const
{
    dest << signature
        << public_key
        << validity_not_before
        << validity_not_after
        << subject_common_name
        << subject_organizational_unit_name
        << subject_organization_name
        << subject_country_name
        << issuer_common_name
        << issuer_organizational_unit_name
        << issuer_organization_name
        << issuer_country_name
        << raw
        << issuer
        << subject
        << subject_locality_name
        << subject_state_or_province_name
        << issuer_locality_name
        << issuer_state_or_province_name;
}

void
CSsl::to_cef(OutputFormatFile &dest, std::string prefix) const
{
	cef_row("cssl", "signature", signature, prefix);
	cef_row("cssl", "public_key", public_key , prefix);
    cef_row("cssl", "validity_not_before", validity_not_before, prefix);
    cef_row("cssl", "validity_not_after", validity_not_after, prefix);
    cef_row("cssl", "subject_common_name", subject_common_name, prefix);

    cef_row("cssl", "subject_organizational_unit_name", subject_organizational_unit_name, prefix);
    cef_row("cssl", "subject_organization_name", subject_organization_name, prefix);
    cef_row("cssl", "subject_country_name", subject_country_name, prefix);

    cef_row("cssl", "issuer_common_name", issuer_common_name, prefix);
    cef_row("cssl", "issuer_organizational_unit_name", issuer_organizational_unit_name, prefix);
    cef_row("cssl", "issuer_organization_name", issuer_organization_name, prefix);

    cef_row("cssl", "issuer_country_name", issuer_country_name, prefix);
    cef_row("cssl", "raw", raw, prefix);
    cef_row("cssl", "issuer", issuer, prefix);

    cef_row("cssl", "subject", subject, prefix);
    cef_row("cssl", "subject_locality_name", subject_locality_name, prefix);
    cef_row("cssl", "subject_state_or_province_name", subject_state_or_province_name, prefix);
    cef_row("cssl", "issuer_locality_name", issuer_locality_name, prefix);
    cef_row("cssl", "issuer_state_or_province_name", issuer_state_or_province_name, prefix);

}

void
CFtp::serialize(Writer &w) const
{
    w.String("cftp");
    w.StartObject();

    SERIAL_ARRAY(user_name);
    SERIAL_ARRAY(password);
    SERIAL_ARRAY(working_directory);
    SERIAL_ARRAY(server_file_name);
    SERIAL_ARRAY(transfer_working_directory);

    w.EndObject();
}

const std::string CFtp::csv_prefix = "cftp";
const std::vector<std::string> CFtp::csv_fields = {
    "user_name",
    "password",
    "working_directory",
    "server_file_name",
    "transfer_working_directory"
};

void
CFtp::to_csv(OutputFormatFile &dest) const
{
    dest << user_name
        << password
        << working_directory
        << server_file_name
        << transfer_working_directory;
}

void
CFtp::to_cef(OutputFormatFile &dest, std::string prefix) const
{
    cef_row("cftp", "user_name", user_name, prefix);
    cef_row("cftp", "password", password, prefix);
    cef_row("cftp", "working_directory", working_directory, prefix);
    cef_row("cftp", "server_file_name", server_file_name, prefix);
    cef_row("cftp", "transfer_working_directory", transfer_working_directory, prefix);
}

void
Dns::serialize(Writer &w) const
{
    if (dnsho.size()) {
        w.String("dnsho"); serialize_array_not_empty(w, dnsho);
    }
    if (dnsip.size()) {
        w.String("dnsip"); serialize_array(w, dnsip);
    }
}

const std::string Dns::csv_prefix = "dns";
const std::vector<std::string> Dns::csv_fields = {
     "host", "ip_address"
};

void
Dns::to_csv(OutputFormatFile &dest) const
{
    char buf[INET_ADDRSTRLEN];
    dest << dnsho;
    std::vector<std::string> transformed_dns_ip;
    std::transform(dnsip.begin(), dnsip.end(),
                   std::back_inserter(transformed_dns_ip),
                   [&buf] (int32_t i) {
                   struct in_addr addr = in_addr{ntohl(i)};
                       return inet_ntop(AF_INET, &addr, buf, sizeof(buf));
                   });
    dest << transformed_dns_ip;
}
void
Dns::to_cef(OutputFormatFile &dest, std::string prefix) const
{
    char buf[INET_ADDRSTRLEN];
    std::vector<std::string> transformed_dns_ip;
    std::transform(dnsip.begin(), dnsip.end(),
                   std::back_inserter(transformed_dns_ip),
                   [&buf] (int32_t i) {
                   struct in_addr addr = in_addr{ntohl(i)};
                       return inet_ntop(AF_INET, &addr, buf, sizeof(buf));
                   });
    cef_row("dns", "host", dnsho, prefix);
    cef_row("dns", "ip_address", transformed_dns_ip, prefix);
}

void
Ftp::serialize(Writer &w) const
{
    w.String("ftp");
    w.StartObject();
    if (username.size()) {
        w.String("u"); serialize_array(w, username);
    }
    if (password.size()) {
        w.String("p"); serialize_array(w, password);
    }
    if (file.size()) {
        w.String("file"); serialize_array(w, file);
    }
    w.EndObject();
}

const std::string Ftp::csv_prefix = "ftp";
const std::vector<std::string> Ftp::csv_fields = {
     "username", "password"
};

void
Ftp::to_csv(OutputFormatFile &dest) const
{
    dest << username << password;
}

void
Ftp::to_cef(OutputFormatFile &dest, std::string prefix) const
{
    cef_row("ftp", "username", username, prefix);
    cef_row("ftp", "password", password, prefix);
}

void
Dhcp::serialize(Writer &w) const
{
    char buf[INET_ADDRSTRLEN];

    w.String("dhcp");
    w.StartObject();
    w.String("ch");
        w.String(utils::get_48bit_string(client_hwaddr).c_str(), sizeof(client_hwaddr));
    w.String("cip"); w.String(inet_ntop(AF_INET, &client_ip, buf, sizeof(buf)));
    w.String("yip"); w.String(inet_ntop(AF_INET, &your_ip, buf, sizeof(buf)));
    w.String("sip"); w.String(inet_ntop(AF_INET, &server_ip, buf, sizeof(buf)));
    w.String("aip"); w.String(inet_ntop(AF_INET, &agent_ip, buf, sizeof(buf)));
    w.String("sm"); w.String(inet_ntop(AF_INET, &subnet_mask, buf, sizeof(buf)));
    std::vector<std::string> vec;
    auto transformer = [&vec, &buf] (std::unordered_set<uint32_t> const &set) {
        vec.clear();
        std::transform(set.cbegin(), set.cend(),
                       std::back_inserter(vec),
                       [&buf] (uint32_t i) {
                       struct in_addr addr = in_addr{(i)};
                           return inet_ntop(AF_INET, &addr, buf, sizeof(buf));
                       });
    };

    if (routers.size()) {
    	transformer(routers);
        w.String("rt"); serialize_array(w, vec);
    }
    if (name_servers.size()) {
    	transformer(name_servers);
        w.String("ns"); serialize_array(w, vec);
    }
    if (dns.size()) {
    	transformer(dns);
        w.String("dns"); serialize_array(w, vec);
    }
    if (hostnames.size()) {
        w.String("hst"); serialize_array(w, hostnames);
    }
    if (domain_names.size()) {
        w.String("dn"); serialize_array(w, domain_names);
    }
    if (class_id.size()) {
        w.String("cid"); serialize_array(w, class_id);
    }
    if (client_id.size()) {
    	vec.clear();
    	std::transform(client_id.begin(), client_id.end(),
    	               std::back_inserter(vec),
    	               [&buf] (uint64_t i) {
    	                   return utils::get_48bit_string(i);
    	               });
        w.String("clid"); serialize_array(w, vec);
    }
    if (cfqdn.size()) {
        w.String("cfqdn"); serialize_array(w, cfqdn);
    }
    w.EndObject();
}

const std::string Dhcp::csv_prefix = "dhcp";
const std::vector<std::string> Dhcp::csv_fields = {
    "client_hwaddr", "client_ip_address", "subnet_mask",
    "your_ip_address", "server_ip_address", "agent_ip_address",
    "routers", "name_servers", "dns_servers", "hostnames",
    "domain_names", "class_ids", "client_ids", "client_fqdn"
};

void
Dhcp::to_csv(OutputFormatFile &dest) const
{
    char buf[INET_ADDRSTRLEN];
    std::vector<std::string> vec;
    auto transformer = [&vec, &buf] (std::unordered_set<uint32_t> const *set) {
        vec.clear();
        std::transform(set->cbegin(), set->cend(),
                       std::back_inserter(vec),
                       [&buf] (uint32_t i) {
                       struct in_addr addr = in_addr{(i)};
                           return inet_ntop(AF_INET, &addr, buf, sizeof(buf));
                       });
    };

    dest << utils::get_48bit_string(client_hwaddr)
        << inet_ntop(AF_INET, &client_ip, buf, sizeof(buf))
        << inet_ntop(AF_INET, &subnet_mask, buf, sizeof(buf))
        << inet_ntop(AF_INET, &your_ip, buf, sizeof(buf))
        << inet_ntop(AF_INET, &server_ip, buf, sizeof(buf))
        << inet_ntop(AF_INET, &agent_ip, buf, sizeof(buf));
    transformer(&routers);
    dest << vec;
    transformer(&name_servers);
    dest << vec;
    transformer(&dns);
    dest << vec;
    vec.clear();
    std::transform(client_id.begin(), client_id.end(),
                   std::back_inserter(vec),
                   [&buf] (uint64_t i) {
                       return utils::get_48bit_string(i);
                   });
    dest << hostnames << domain_names << class_id << vec << cfqdn;
}

void
Dhcp::to_cef(OutputFormatFile &dest, std::string prefix) const
{
    char buf[INET_ADDRSTRLEN];
    std::vector<std::string> vec;
    auto transformer = [&vec, &buf] (std::unordered_set<uint32_t> const *set) {
        vec.clear();
        std::transform(set->cbegin(), set->cend(),
                       std::back_inserter(vec),
                       [&buf] (int32_t i) {
                       struct in_addr addr = in_addr{ntohl(i)};
                           return inet_ntop(AF_INET, &addr, buf, sizeof(buf));
                       });
    };

    cef_row("dhcp", "client_hwaddr", utils::get_48bit_string(client_hwaddr), prefix);

	cef_row("dhcp", "client_ip_address", std::string(inet_ntop(AF_INET, &client_ip, buf, sizeof(buf))), prefix);
    cef_row("dhcp", "subnet_mask", std::string(inet_ntop(AF_INET, &subnet_mask, buf, sizeof(buf))), prefix);
    cef_row("dhcp", "your_ip_address", std::string(inet_ntop(AF_INET, &your_ip, buf, sizeof(buf))), prefix);
    cef_row("dhcp", "server_ip_address", std::string(inet_ntop(AF_INET, &server_ip, buf, sizeof(buf))), prefix);
    cef_row("dhcp", "agent_ip_address", std::string(inet_ntop(AF_INET, &agent_ip, buf, sizeof(buf))), prefix);
    cef_row("dhcp", "agent", std::string(inet_ntop(AF_INET, &agent_ip, buf, sizeof(buf))), prefix);

    transformer(&routers);
    dest << prefix + "dhcp.routers=" << vec;
    transformer(&name_servers);
    dest << prefix + "dhcp.servers=" << vec;
    transformer(&dns);
    dest << prefix + "dhcp.dns_servers=" << vec;
    vec.clear();
    vec.insert(vec.begin(), client_id_str.begin(), client_id_str.end());
    std::transform(client_id.begin(), client_id.end(),
                   std::back_inserter(vec),
                   [&buf] (uint64_t i) {
                       return utils::get_48bit_string(i);
                   });

    cef_row("dhcp", "hostnames", hostnames,prefix);

    cef_row("dhcp", "domain_names", domain_names, prefix);
    cef_row("dhcp", "class_ids", class_id, prefix);
    cef_row("dhcp", "clients_ids", vec, prefix);
    cef_row("dhcp", "client_fqdn", cfqdn, prefix);
}

void
Sip::serialize(Writer &w) const
{
    w.String("sip");
    w.StartObject();
    if (from_username.size()) {
        w.String("fu");         serialize_array(w, from_username);
    }
    if (from_userid.size()) {
        w.String("fuid");       serialize_array(w, from_userid);
    }
    if (to_username.size()) {
        w.String("tu");         serialize_array(w, to_username);
    }
    if (to_userid.size()) {
        w.String("tuid");       serialize_array(w, to_userid);
    }
    if (mediatype.size()) {
        w.String("m");          serialize_array(w, mediatype);
    }
    w.EndObject();
}

const std::string Sip::csv_prefix = "sip";
const std::vector<std::string> Sip::csv_fields = {
     "from", "from_id", "to", "to_id", "media_type"
};

void
Sip::to_csv(OutputFormatFile &dest) const
{
    dest << from_username << from_username << to_username
         << to_userid << mediatype;
}

void
Sip::to_cef(OutputFormatFile &dest, std::string prefix) const
{
    cef_row("sip", "from", from_username, prefix);
    cef_row("sip", "from_id", from_username, prefix);
    cef_row("sip", "to", to_username, prefix);
    cef_row("sip", "to_id", to_userid, prefix);
    cef_row("sip", "media_type", mediatype, prefix);
}

const std::string File::csv_prefix = "file";
const std::vector<std::string> File::csv_fields = {
     "name", "path", "content-type", "size"
};

void
File::to_csv(OutputFormatFile &dest) const
{
    dest << name
         << rpath
         << content_type;
     if (size != std::numeric_limits<uint64_t>::max()) {
         dest << size;
     } else {
         dest << written;
     }
}

void
File::to_cef(OutputFormatFile &dest, std::string prefix) const
{
	cef_row("file", "name", name, prefix);
	cef_row("file", "path", rpath, prefix);
	cef_row("file", "content-type", content_type, prefix);

     if (size != std::numeric_limits<uint64_t>::max()) {
		 dest << prefix + "file.size=" << size << " ";
	 } else {
		 dest << prefix + "file.size=" << written << "";
     }
}
void
File::serialize(Writer &w) const
{
    w.StartObject();
    w.String("n");        w.String(name.c_str());
    w.String("p");        w.String(rpath.c_str());
    w.String("l");        w.String(location.c_str());
    w.String("ct");       w.String(content_type.c_str());
    w.String("state");
    switch (state) {
        case file_state::OK:
            w.String("o");
            break;
        case file_state::COMPRESSED:
            w.String("s");
            break;
        case file_state::DROPPED:
            w.String("d");
            break;
        default:
            w.String("u");
            break;
    }
    w.String("m");        w.String(method.c_str());
    if (size != std::numeric_limits<uint64_t>::max()) {
        w.String("s");    w.Uint(size);
    } else {
        w.String("s");    w.Uint(written);
    }

    if (is_gps) {
        w.String("gps");
        w.StartObject();
        w.String("la");        w.Double(gps_lat);
        w.String("lo");        w.Double(gps_lon);
        w.EndObject();
    }

    if (not cam_manufacturer.empty() or not cam_model.empty()) {
        w.String("cam");
        w.StartObject();
        w.String("ma");        w.String(cam_manufacturer.c_str());
        w.String("mo");        w.String(cam_model.c_str());
        w.EndObject();
    }

    if (not created.empty()) {
        w.String("c");        w.String(created.c_str());
    }

    if (not range.empty()) {
        w.String("r");        w.String(range.c_str());
    }

    if (offset >= 0) {
    w.String("of");        w.Int(offset);
    }
    w.EndObject();
}

void
Gtp::serialize(Writer &w) const
{
    w.String("gtp");
    w.StartObject();
    w.String("s");        serialize_array(w, imsi);
    w.String("e");        serialize_array(w, imei);
    w.String("l");        serialize_array(w, lac);
    w.String("r");        serialize_array(w, rac);
    w.String("m");        serialize_array(w, msisdn);
    w.EndObject();
}

const std::string Gtp::csv_prefix = "gtp";
const std::vector<std::string> Gtp::csv_fields = {
     "msisdn", "imsi", "imei", "lac", "rac"
};

void
Gtp::to_csv(OutputFormatFile &dest) const
{
    dest << msisdn << imsi << imei << lac << rac;
}

void
Gtp::to_cef(OutputFormatFile &dest, std::string prefix) const
{

	cef_row("gtp", "msisdn", msisdn, prefix);
	cef_row("gtp", "imsi", imsi, prefix);
	cef_row("gtp", "imei", imei, prefix);
	cef_row("gtp", "lac", lac, prefix);
	cef_row("gtp", "rac", rac, prefix);
}

void
Radius::serialize(Writer &w) const
{
    w.String("radius");
    w.StartObject();
    w.String("u");        serialize_array(w, username);
    w.String("p");        serialize_array(w, password);
    w.String("ni");       serialize_array(w, nasip);
    w.String("np");       serialize_array(w, nasport);
    w.String("ce");       serialize_array(w, called_stid);
    w.String("ci");       serialize_array(w, calling_stid);
    w.EndObject();
}

const std::string Radius::csv_prefix = "radius";
const std::vector<std::string> Radius::csv_fields = {
     "nas_ip", "nas_port", "called_station_id", "calling_station_id",
     "username", "password"

};

void
Radius::to_csv(OutputFormatFile &dest) const
{
    char buf[INET_ADDRSTRLEN];
    std::vector<std::string> transformed_ip;
    std::transform(nasip.begin(), nasip.end(),
                   std::back_inserter(transformed_ip),
                   [&buf] (int32_t i) {
                   struct in_addr addr = in_addr{ntohl(i)};
                       return inet_ntop(AF_INET, &addr, buf, sizeof(buf));
                   });
    dest << transformed_ip
         << nasport
         << called_stid
         << calling_stid
         << username
         << password;
}

void
Radius::to_cef(OutputFormatFile &dest, std::string prefix) const
{
    char buf[INET_ADDRSTRLEN];
    std::vector<std::string> transformed_ip;
    std::transform(nasip.begin(), nasip.end(),
                   std::back_inserter(transformed_ip),
                   [&buf] (int32_t i) {
                   struct in_addr addr = in_addr{ntohl(i)};
                       return inet_ntop(AF_INET, &addr, buf, sizeof(buf));
                   });
	cef_row("radius", "nas_ip", transformed_ip, prefix);
	cef_row("radius", "nas_port", nasport, prefix);
	cef_row("radius", "called_station_id", called_stid, prefix);
	cef_row("radius", "calling_station_id", calling_stid, prefix);
	cef_row("radius", "user_name", username, prefix);
	cef_row("radius", "password", password, prefix);
}

void
Diameter::serialize(Writer &w) const
{
    w.String("diameter");
    w.StartObject();
    w.String("u");        serialize_array(w, username);
    w.String("p");        serialize_array(w, password);
    w.String("ni");       serialize_array(w, nasip);
    w.String("np");       serialize_array(w, nasport);
    w.String("ce");       serialize_array(w, nas_ceid);
    w.String("ci");       serialize_array(w, nas_ciid);
    w.String("i");        serialize_array(w, nasid);
    w.EndObject();
}

const std::string Diameter::csv_prefix = "diameter";
const std::vector<std::string> Diameter::csv_fields = {
    "nas_ip", "nas_port", "called_station_id",
    "calling_station_id", "nas_id", "username", "password"
};

void
Diameter::to_csv(OutputFormatFile &dest) const
{
    char buf[INET_ADDRSTRLEN];
    std::vector<std::string> transformed_ips;
    std::transform(nasip.begin(), nasip.end(),
                   std::back_inserter(transformed_ips),
                   [&buf] (int32_t i) {
                   struct in_addr addr = in_addr{ntohl(i)};
                       return inet_ntop(AF_INET, &addr, buf, sizeof(buf));
                   });
    dest << transformed_ips
         << nasport
         << nas_ceid
         << nas_ciid
         << nasid
         << username
         << password;
}

void
Diameter::to_cef(OutputFormatFile &dest, std::string prefix) const
{
    char buf[INET_ADDRSTRLEN];
    std::vector<std::string> transformed_ips;
    std::transform(nasip.begin(), nasip.end(),
                   std::back_inserter(transformed_ips),
                   [&buf] (int32_t i) {
                   struct in_addr addr = in_addr{ntohl(i)};
                       return inet_ntop(AF_INET, &addr, buf, sizeof(buf));
                   });


	cef_row("diameter", "nas_ip", transformed_ips, prefix);
	cef_row("diameter", "nas_port", nasport, prefix);
	cef_row("diameter", "called_staiton_id", nas_ceid, prefix);
	cef_row("diameter", "calling_station_ip", nas_ciid, prefix);
	cef_row("diameter", "nas_id", nasid, prefix);
	cef_row("diameter", "username", username, prefix);
	cef_row("diameter", "password", password, prefix);
}

void
Socks::serialize(Writer &w) const
{
    w.String("socks");
    w.StartObject();
    w.String("u");        serialize_array(w, username);
    w.String("ui");       serialize_array(w, userid);
    w.String("p");        serialize_array(w, password);
    w.String("d");        serialize_array(w, domain);
    w.EndObject();
}

const std::string Socks::csv_prefix = "socks";
const std::vector<std::string> Socks::csv_fields = {
    "user_id", "username", "password", "domain"
};

void
Socks::to_csv(OutputFormatFile &dest) const
{
    dest << username << userid << password << domain;
}

void
Socks::to_cef(OutputFormatFile &dest, std::string prefix) const
{
	cef_row("socks", "username", username, prefix);
	cef_row("socks", "user_id", userid, prefix);
	cef_row("socks", "password", password, prefix);
	cef_row("socks", "domain", domain, prefix);
}

void
Jabber::serialize(Writer &w) const
{
    w.String("jabber");
    w.StartObject();
    w.String("u");        serialize_array(w, userid);
    w.String("bi");       serialize_array(w, buddiesid);
    w.String("f");        serialize_array(w, from);
    w.String("t");        serialize_array(w, to);
    w.String("m");        serialize_array(w, message);
    w.String("s");        serialize_array(w, status);
    w.String("r");        serialize_array(w, roomid);
    w.String("si");       serialize_array(w, sessionid);
    w.String("fn");       serialize_array(w, filename);
    w.EndObject();
}

const std::string Jabber::csv_prefix = "jabber";
const std::vector<std::string> Jabber::csv_fields = {
    "user_id", "buddies_id", "from", "to", "message", "status", "room_id",
    "session_id", "filename"
};

void
Jabber::to_csv(OutputFormatFile &dest) const
{
    dest << userid
         << buddiesid
         << from
         << to
         << message
         << status
         << roomid
         << sessionid
         << filename;
}

void
Jabber::to_cef(OutputFormatFile &dest, std::string prefix) const
{
	cef_row("jabber", "user_id", userid, prefix);
	cef_row("jabber", "buddies_id", buddiesid, prefix);
	cef_row("jabber", "from", from, prefix);
	cef_row("jabber", "to", to, prefix);
	cef_row("jabber", "message", message, prefix);

	cef_row("jabber", "status", status, prefix);
	cef_row("jabber", "room_id", roomid, prefix);
	cef_row("jabber", "session_id", sessionid, prefix);
	cef_row("jabber", "filename", filename, prefix);
}

void
Ssl::serialize(Writer &w) const
{
    w.String("ssl");
    w.StartObject();
    w.String("sn");       serialize_array(w, servname);
    w.String("c");        serialize_array(w, country);
    w.String("p");        serialize_array(w, postalcode);
    w.String("s");        serialize_array(w, state);
    w.String("l");        serialize_array(w, locality);
    w.String("a");        serialize_array(w, address);
    w.String("o");        serialize_array(w, org);
    w.String("ou");       serialize_array(w, orgunit);
    w.String("cn");       serialize_array(w, commonname);
    w.EndObject();
}

const std::string Ssl::csv_prefix = "ssl";
const std::vector<std::string> Ssl::csv_fields = {
    "server_name", "country", "zipcode", "state", "locality", "address",
    "organization", "organization_unit", "server_common_name"
};

void
Ssl::to_csv(OutputFormatFile &dest) const
{
    dest << servname
         << country
          << postalcode
          << state
          << locality
          << address
          << org
          << orgunit
          << commonname;
}

void
Ssl::to_cef(OutputFormatFile &dest, std::string prefix) const
{
	cef_row("ssl", "server_name", servname, prefix);
	cef_row("ssl", "country", country, prefix);
	cef_row("ssl", "zipcode", postalcode, prefix);
	cef_row("ssl", "state", state , prefix);
	cef_row("ssl", "locality", locality, prefix);
	cef_row("ssl", "address", address, prefix);
	cef_row("ssl", "organization", org, prefix);
	cef_row("ssl", "organization_unit", orgunit , prefix);
	cef_row("ssl", "server_common_name", commonname, prefix);
}

void
Nat::serialize(Writer &w) const
{
    w.String("nat");
    w.StartObject();
    w.String("o");        serialize_array(w, os);
    w.String("s");        w.Bool(src_nat);
    w.String("d");        w.Bool(dst_nat);
    w.EndObject();
}

const std::string Nat::csv_prefix = "nat";
const std::vector<std::string> Nat::csv_fields = {
    "os"
};

void
Nat::to_csv(OutputFormatFile &dest) const
{
    dest << os;
}

void
Nat::to_cef(OutputFormatFile &dest, std::string prefix) const
{
	cef_row("nat", "os", os, prefix);
}

void
Bittorrent::serialize(Writer &w) const
{
    w.String("bittorrent");
    w.StartObject();
    w.String("o");        serialize_array(w, m_os);
    w.EndObject();
}

void
Bittorrent::to_csv(OutputFormatFile &) const
{
}

void
Bittorrent::to_cef(OutputFormatFile &, std::string) const
{
}

void
WhatsApp::serialize(Writer &w) const
{
    w.String("whatsapp");
    w.StartObject();
    if (phone_number.size()) { w.String("n1"); w.String(phone_number.c_str()); }
    if (phone_type.size()) { w.String("d");  w.String(phone_type.c_str()); }
    w.EndObject();
}

const std::string WhatsApp::csv_prefix = "whatsapp";
const std::vector<std::string> WhatsApp::csv_fields = {
    "msisdn", "device_id"
};

void
WhatsApp::to_csv(OutputFormatFile &dest) const
{
    dest << phone_number
        << phone_type;
}

void
WhatsApp::to_cef(OutputFormatFile &dest, std::string prefix) const
{
	cef_row("whatapp", "phone_number", phone_number, prefix);
	cef_row("whatapp", "phone_type", phone_type, prefix);
}

void
Ldap::serialize(Writer &w) const
{
    w.String("ldap");
    w.StartObject();
    if (dns_host_name.size()) { w.String("dns"); w.String(dns_host_name.c_str()); }
    w.EndObject();
}

const std::string Ldap::csv_prefix = "ldap";
const std::vector<std::string> Ldap::csv_fields = {
    "dns_host_name"
};

void
Ldap::to_csv(OutputFormatFile &dest) const
{
    dest << dns_host_name;
}

void
Ldap::to_cef(OutputFormatFile &dest, std::string prefix) const
{
	cef_row("ldap", "dns_host_name", dns_host_name, prefix);
}

void
Telnet::serialize(Writer &w) const
{
    w.String("telnet");
    w.StartObject();

    w.String("firstmessage");
    w.String(firstmessage.c_str());

    w.String("username");
    w.String(username.c_str());

    w.String("password");
    w.String(password.c_str());

    w.EndObject();
}

const std::string Telnet::csv_prefix = "telnet";
const std::vector<std::string> Telnet::csv_fields = {
	"firstmessage",
	"username",
	"password",
};

void
Telnet::to_csv(OutputFormatFile &dest) const
{
    dest << firstmessage
		 << username
		 << password;

}
void
Telnet::to_cef(OutputFormatFile &dest, std::string prefix) const
{

	cef_row("telnet", "firstmessage", firstmessage, prefix);
	cef_row("telnet", "username", username, prefix);
	cef_row("telnet", "password", password, prefix);
}


void
Smb::serialize(Writer &w) const
{
    w.String("smb");
    w.StartObject();

    w.String("domain");
    w.String(smb_domain.c_str());

    w.String("user");
    w.String(smb_user.c_str());

    w.String("host");
    w.String(smb_host.c_str());

    w.EndObject();
}

const std::string Smb::csv_prefix = "smb";
const std::vector<std::string> Smb::csv_fields = {
    "domain",
    "user",
    "host",
	"mainevents"
};

void
Smb::to_cef(OutputFormatFile &dest, std::string prefix) const
{
	cef_row("smb", "domain", smb_domain, prefix);
	cef_row("smb", "user", smb_user, prefix);
	cef_row("smb", "host", smb_host, prefix);
	cef_row("smb", "mainevents", smb_main_events, prefix);
}

void
Smb::to_csv(OutputFormatFile &dest) const
{
  dest  << smb_domain
        << smb_user
        << smb_host
		<< smb_main_events;
}

void
Krb5::serialize(Writer &w) const
{
    w.String("krb5");
    w.StartObject();
    w.String("cname");
    w.String(cname.c_str());
    w.EndObject();
}

const std::string Krb5::csv_prefix = "krb5";
const std::vector<std::string> Krb5::csv_fields = {
    "cname"
};

void
Krb5::to_csv(OutputFormatFile &dest) const
{
  dest  << cname;
}

void
Krb5::to_cef(OutputFormatFile &dest, std::string prefix) const
{
	cef_row("krb5", "cname", cname, prefix);
}

const char *
categoryName(event_category const c)
{
    switch (c) {
        case event_category::message_post:
            return("messagePost");
        case event_category::message_read:
            return("messageRead");
        case event_category::message_one2one:
            return("messagePrivate");
        case event_category::command:
            return("command");
        case event_category::filetransfer:
            return("filetransfer");
        case event_category::encrypted:
            return("encrypted");
        case event_category::login:
            return("login");
        case event_category::session_setup:
            return("session setup");
        case event_category::tree_connect:
            return("tree connect");
        case event_category::create_directory:
            return("create directory");
        case event_category::create_file:
            return("create file");
        case event_category::read:
            return("read file");
        case event_category::write:
            return("write file");
        case event_category::close:
            return("close file");
        case event_category::delete_file:
            return("delete file");
        case event_category::delete_directory:
            return("delete directory");
        case event_category::mail:
            return("mail");
        case event_category::voip:
            return("voip");
        case event_category::http_transaction:
            return("http_transaction");
        case event_category::http_request_data_transfer:
            return("http_request_data_transfer");
        case event_category::http_response_data_transfer:
            return("http_response_data_transfer");
        case event_category::http_transaction_done:
            return("http_transaction_done");
        case event_category::cssl_key_exchange:
            return("cssl_key_exchange");
        case event_category::cssl_client_hello:
            return("cssl_client_hello");
        case event_category::cssl_certificate:
            return("cssl_certificate");
        case event_category::chat_user:
            return("chat user");
        case event_category::chat_user_info:
            return("chat user info");
        case event_category::chat_transfer:
            return("chat transfer");
        case event_category::chat_transfer_announcement:
            return("chat transfer announcement");
        case event_category::chat_message:
            return("chat message");
        case event_category::chatroom:
            return("chatroom");
        case event_category::chat_other:
            return("chat other");
        case event_category::cftp_session:
            return("cftp session");
        case event_category::cftp_transfer:
            return("cftp transfer");
        case event_category::cftp_transfer_data:
            return("cftp transfer data");
        case event_category::undefined: //intentional fall-through
        default:
            return("undefined");
    };
}

void
Event::serialize(Writer &w) const
{
    w.StartObject();
    w.String("category");    w.String(categoryName(category));
    w.String("timestamp");   w.Uint64(timestamp);
    w.String("value");       w.String(value.c_str());
    w.String("direction");   w.Bool(direction);
    w.EndObject();
}

void
Event::to_csv(OutputFormatFile &) const
{

}

void
Event::to_cef(OutputFormatFile &, std::string) const
{

}
SessionInfo::~SessionInfo()
{
    for (auto& p: protocol_infos) {
        delete p.second;
    }
}

void
SessionInfo::serialize(Writer &w) const
{
    w.StartObject();

    w.String("nodeid"); w.String(GlobalConfig::get().output.module.json.dmi.c_str());
    w.String("id"); w.String(id.c_str());

    w.String("prot-term");
    w.StartArray();
    std::unordered_set<std::string> seen_protocols;
    for (auto &&i: prot_term) {
        if (i != PACE2_PROTOCOL_LAYER3_IPV4
                and i != PACE2_PROTOCOL_LAYER3_IPV6
                and i != PACE2_PROTOCOL_UNKNOWN) {
            auto tmp = pace2_get_protocol_long_str(i);
            if (seen_protocols.find(tmp) == seen_protocols.end()) {
                seen_protocols.emplace(tmp);
                serializable::serialize(w, tmp);
            }
        }
    }
    if (cdc_matched != odc::custom::cdcTypes::cdc_no_match) {
        char const *ptr = odc::custom::cdc_info[(int)cdc_matched].protocol;
        if (ptr[0]) {
            if (seen_protocols.find(ptr) == seen_protocols.end()) {
                seen_protocols.emplace(ptr);
                serializable::serialize(w, ptr);
            }
        }
    }
    w.EndArray();

    if (prot_attr[0] != PACE2_PROTOCOL_ATTRIBUTE_NONE) {
        w.String("prot-attr");
        serialize_array_if_not(w, prot_attr, PACE2_PROTOCOL_ATTRIBUTE_NONE);
    }

    if (app != PACE2_APPLICATION_UNKNOWN) {
        w.String("app"); serializable::serialize(w, app);
    }
    else if (cdc_matched != odc::custom::cdcTypes::cdc_no_match) {
        char const *ptr = odc::custom::cdc_info[(int)cdc_matched].application;
        if (ptr[0]) {
            w.String("app");
            w.String(ptr);
        }
    }

    if (app_attr[0] != PACE2_APPLICATION_ATTRIBUTE_NONE) {
        w.String("app-attr");
        serialize_array_if_not(w, app_attr, PACE2_APPLICATION_ATTRIBUTE_NONE);
    }
    char buf[INET6_ADDRSTRLEN];


    w.String("ipv6"); w.Bool(flow_tuple.is_ip_v6);
    if (flow_tuple.is_ip_v6) {
        w.String("ip1");
        w.String(inet_ntop(AF_INET6, &flow_tuple.client_ip.ipv6,
                    buf, INET6_ADDRSTRLEN));
        w.String("ip2");
        w.String(inet_ntop(AF_INET6, &flow_tuple.server_ip.ipv6,
                    buf, INET6_ADDRSTRLEN));
    }
    else {
        auto tmp = flow_tuple.client_ip.ipv4;
        tmp.s_addr = ntohl(tmp.s_addr);

        w.String("ip1");
        w.String(inet_ntop(AF_INET, &tmp, buf, INET_ADDRSTRLEN));

        tmp = flow_tuple.server_ip.ipv4;
        tmp.s_addr = ntohl(tmp.s_addr);

        w.String("ip2");
        w.String(inet_ntop(AF_INET, &tmp, buf, INET_ADDRSTRLEN));

        //w.String("a1"); w.Uint(flow_tuple.client_ip.ipv4.s_addr);
        //w.String("a2"); w.Uint(flow_tuple.server_ip.ipv4.s_addr);
    }
    w.String("p1"); w.Uint(flow_tuple.client_port);
    w.String("p2"); w.Uint(flow_tuple.server_port);
    w.String("by"); w.Uint(nb_bytes_src + nb_bytes_dst);
    w.String("db"); w.Uint(nb_data_bytes_src + nb_data_bytes_dst);
    w.String("by1"); w.Uint(nb_bytes_src);
    w.String("db1"); w.Uint(nb_data_bytes_src);
    w.String("by2"); w.Uint(nb_bytes_dst);
    w.String("db2"); w.Uint(nb_data_bytes_dst);
    //w.String("fp"); w.Uint(first_packet_timestamp / 1000);
    w.String("fpd"); w.Int64(first_packet_timestamp);
    //w.String("lp"); w.Uint(last_packet_timestamp / 1000);
    w.String("lpd"); w.Int64(last_packet_timestamp);
    if (os.size() or os_version.size()) {
        w.String("os");
        w.StartObject();
        w.String("n"); w.String(os.c_str());
        w.String("v"); w.String(os_version.c_str());
        w.EndObject();
    }
    if (geoloc_src_ip.size()) {
        w.String("g1"); w.String(geoloc_src_ip.c_str());
    }
    if (geoloc_dst_ip.size()) {
        w.String("g2"); w.String(geoloc_dst_ip.c_str());
    }

    char mac[ETHER_STRING_SIZE] = {0};
    ETHER_TO_STRING(mac1, mac);
    w.String("mac1-term"); w.StartArray(); w.String(mac); w.EndArray();
    ETHER_TO_STRING(mac2, mac);
    w.String("mac2-term"); w.StartArray(); w.String(mac); w.EndArray();

    if (vlan_id > 0) {
         w.String("vlan"); w.Int(vlan_id);
    }

    w.String("pa"); w.Uint64(nb_packets_src + nb_packets_dst);
    w.String("pa1"); w.Uint64(nb_packets_src);
    w.String("pa2"); w.Uint64(nb_packets_dst);
    w.String("case"); w.Uint(case_id);
    w.String("pcap"); w.String(pcap_id.c_str());
    if (not packets_offsets.empty()) {
        w.String("po"); serialize_array(w, packets_offsets);
    }
    for (auto& p: protocol_infos)
        p.second->serialize(w);
    if (files.size()) {
        w.String("files");
        w.StartArray();
        for (auto &i: files) {
            i.second.serialize(w);
        }
        w.EndArray();
    }
    w.String("gps");      w.Bool(is_gps);

    //Set marker for bad flows. Set if tcp_lost_bytes <> 0 or the tcp_wo_hs
    //attribute is set
    bool bad_flow = false;
    if (lost_bytes > 0) {
        bad_flow = true;
    } else {
        for (unsigned int i = 0; i < proto_max_attrs; ++i) {
            if (prot_attr[i] == PACE2_PROTOCOL_ATTRIBUTE_TCP_WO_HANDSHAKE) {
                bad_flow = true;
                break;
            }
        }
    }
    w.String("bf"); w.Bool(bad_flow);
    w.String("tlb"); w.Uint64(lost_bytes);

    w.String("events");
    w.StartArray();
    for (const auto &ev : events) ev.serialize(w);
    w.EndArray();
    w.EndObject();
}

void
SessionInfo::dump_header(OutputFormatFile &dest)
{
    dest << "id.case"
        << "id.pcap"
        << "id.session"
        << "flow.timestamp"
        << "flow.timestamp_end"
        << "flow.mac_src"
        << "flow.mac_dst"
        << "flow.vlan"
        << "flow.ip_address_src"
        << "flow.port_src"
        << "flow.ip_address_dst"
        << "flow.port_dst"
        << "flow.packets"
        << "flow.packets_src"
        << "flow.packets_dst"
        << "flow.bytes"
        << "flow.bytes_src"
        << "flow.bytes_dst"
        << "flow.payload_bytes"
        << "flow.payload_bytes_src"
        << "flow.payload_bytes_dst"
        << "flow.tcp_bytes_lost"
        << "session.country_src"
        << "session.country_dst"
        << "session.is_gps";
    for (unsigned int i = 0; i < proto_stack_max_length; ++i) {
        dest << (std::string("session.protocol_") + std::to_string(i));
    }
    for (unsigned int i = 0; i < proto_max_attrs; ++i) {
        dest << (std::string("session.protocol_attr_") + std::to_string(i));
    }
    dest << "session.application";
    for (unsigned int i = 0; i < app_max_attrs; ++i) {
        dest << (std::string("session.application_attr_") + std::to_string(i));
    }
    dest << "session.os"
        << "session.os_version";
}

void
SessionInfo::to_csv(OutputFormatFile &dest) const
{
    dest << case_id
        << pcap_id
        << id

        << first_packet_timestamp
        << last_packet_timestamp;

    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
        mac1[0], mac1[1], mac1[2], mac1[3], mac1[4], mac1[5]);
    dest << mac_str;
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
        mac2[0], mac2[1], mac2[2], mac2[3], mac2[4], mac2[5]);
    dest << mac_str
        << (vlan_id > 0 ? std::to_string(vlan_id) : "");

    char buf[INET6_ADDRSTRLEN];
    if (flow_tuple.is_ip_v6) {
        dest << inet_ntop(AF_INET6, &flow_tuple.client_ip.ipv6, buf, INET6_ADDRSTRLEN);
        dest << (flow_tuple.client_port > 0 ? std::to_string(flow_tuple.client_port) : "");
        dest << inet_ntop(AF_INET6, &flow_tuple.server_ip.ipv6, buf, INET6_ADDRSTRLEN);
        dest << (flow_tuple.server_port > 0 ? std::to_string(flow_tuple.server_port) : "");
    }
    else {
        auto tmp = flow_tuple.client_ip.ipv4;
        tmp.s_addr = ntohl(tmp.s_addr);

        dest << inet_ntop(AF_INET, &tmp, buf, INET_ADDRSTRLEN);
        dest << (flow_tuple.client_port > 0 ? std::to_string(flow_tuple.client_port) : "");

        tmp = flow_tuple.server_ip.ipv4;
        tmp.s_addr = ntohl(tmp.s_addr);
        dest << inet_ntop(AF_INET, &tmp, buf, INET_ADDRSTRLEN);
        dest << (flow_tuple.server_port > 0 ? std::to_string(flow_tuple.server_port) : "");
    }
    dest << (nb_packets_src + nb_packets_dst)
        << nb_packets_src
        << nb_packets_dst
        << (nb_bytes_src + nb_bytes_dst)
        << nb_bytes_src
        << nb_bytes_dst
        << (nb_data_bytes_src + nb_data_bytes_dst)
        << nb_data_bytes_src
        << nb_data_bytes_dst
        << lost_bytes
        << geoloc_src_ip
        << geoloc_dst_ip
        << is_gps;

    for (unsigned int i = 0; i < proto_stack_max_length; ++i) {
        if (i == 3 and cdc_matched != odc::custom::cdcTypes::cdc_no_match) {
            char const *ptr = odc::custom::cdc_info[(int)cdc_matched].protocol;
            if (ptr[0]) {// If the string is empty, no custom output
                dest << ptr;
            }
            else {
                dest << pace2_get_protocol_long_str(prot_term[i]);
            }
        }
        else {
            dest << pace2_get_protocol_long_str(prot_term[i]);
        }
    }
    for (unsigned int i = 0; i < proto_max_attrs; ++i) {
        dest << pace2_get_protocol_attribute_str(prot_attr[i]);
    }

    if (app != PACE2_APPLICATION_UNKNOWN) {
        dest << pace2_get_application_long_str(app);
    }
    else if (cdc_matched != odc::custom::cdcTypes::cdc_no_match) {
        char const *ptr = odc::custom::cdc_info[(int)cdc_matched].application;
        if (ptr[0]) {// If the string is empty, no custom output
            dest << ptr;
        }
        else {
            dest << "";
        }
    }
    else {
        dest << "";
    }
    for (unsigned int i = 0; i < app_max_attrs; ++i) {
        dest << pace2_get_application_attribute_str(app_attr[i]);
    }

    dest << os
        << os_version;
}

void
SessionInfo::to_leef(OutputFormatFile &dest, std::string header_row) const
{
    dest << "LEEF:2.0|" + header_row + "|" + std::to_string(case_id) + "|";


	if (strcmp(pace2_get_protocol_long_str(prot_term[3]), "unknown") == 0) {
		if (strcmp(pace2_get_protocol_long_str(prot_term[2]), "unknown") != 0)
            dest << "app=" << OutputFormatFile::NoSep() << pace2_get_protocol_long_str(prot_term[3]);
	} else {
		dest << "app=" << OutputFormatFile::NoSep() << pace2_get_protocol_long_str(prot_term[3]);
	}

	std::time_t t = std::time(nullptr);
	char mbstr[30];
	if (std::strftime(mbstr, sizeof(mbstr), "%b %d %T", std::localtime(&t))) {
		dest << "devTime=" <<OutputFormatFile::NoSep() <<  reinterpret_cast<const char *>(mbstr);
//		dest << "devTimeFormat=MMM dd yyyy HH:mm:ss";
	}
	if (strcmp(pace2_get_protocol_long_str(prot_term[1]), "UDP") == 0 or
	strcmp(pace2_get_protocol_long_str(prot_term[1]), "TCP") == 0) {
		dest << "proto=" <<OutputFormatFile::NoSep() <<  pace2_get_protocol_long_str(prot_term[1]);
	}

	dest << "sev=0";
    char buf[INET6_ADDRSTRLEN];
    dest << "src=" << OutputFormatFile::NoSep() << inet_ntop(AF_INET, &flow_tuple.client_ip.ipv4, buf, INET_ADDRSTRLEN);
    dest << "dst=" << OutputFormatFile::NoSep() << inet_ntop(AF_INET, &flow_tuple.client_ip.ipv4, buf, INET_ADDRSTRLEN);

    dest << "srcPort=" << OutputFormatFile::NoSep() << flow_tuple.client_port;
    dest << "dstPort=" << OutputFormatFile::NoSep() << flow_tuple.server_port;

	// NO NAT stuff
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
        mac1[0], mac1[1], mac1[2], mac1[3], mac1[4], mac1[5]);
    dest << "srcMAC=" << OutputFormatFile::NoSep() << mac_str;

    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
        mac2[0], mac2[1], mac2[2], mac2[3], mac2[4], mac2[5]);

    dest << "dstMAC=" << OutputFormatFile::NoSep() << mac_str;

    dest << "srcBytes=" << OutputFormatFile::NoSep() << nb_bytes_src;
    dest << "dstBytes=" << OutputFormatFile::NoSep() << nb_bytes_dst;
    dest << "srcPackets=" << OutputFormatFile::NoSep() << nb_packets_src;
    dest << "dstPackets=" << OutputFormatFile::NoSep() << nb_packets_dst;
    dest << "totalPackets=" << OutputFormatFile::NoSep()
        << (nb_packets_src + nb_packets_dst);
}

void
SessionInfo::to_cef(OutputFormatFile &dest, unsigned int count, std::string header_row) const
{
	std::time_t t = std::time(nullptr);
	char mbstr[30];
    if (std::strftime(mbstr, sizeof(mbstr), "%b %d %T", std::localtime(&t))) {
    	dest << reinterpret_cast<const char *>(mbstr) << " host CEF:0|" + header_row + "|100|flow|0|";
	}

	if (strcmp(pace2_get_protocol_long_str(prot_term[3]), "unknown") == 0) {
		if (strcmp(pace2_get_protocol_long_str(prot_term[2]), "unknown") != 0)
			dest << "app=" << OutputFormatFile::NoSep() << pace2_get_protocol_long_str(prot_term[2]);
		else
			dest << "app=" << OutputFormatFile::NoSep() << pace2_get_protocol_long_str(prot_term[1]);
	} else {
		dest << "app=" << OutputFormatFile::NoSep() << pace2_get_protocol_long_str(prot_term[3]);
	}

	dest << "cnt=" << OutputFormatFile::NoSep() << count;
    char buf[INET6_ADDRSTRLEN];
    dest << "dst=" << OutputFormatFile::NoSep() << inet_ntop(AF_INET, &flow_tuple.client_ip.ipv4, buf, INET_ADDRSTRLEN);
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
        mac1[0], mac1[1], mac1[2], mac1[3], mac1[4], mac1[5]);
    dest << "dmac=" << OutputFormatFile::NoSep() << mac_str;
    dest << "dpt=" << OutputFormatFile::NoSep() << flow_tuple.server_port;
    dest << "end=" << OutputFormatFile::NoSep() << last_packet_timestamp;
    dest << "in=" << OutputFormatFile::NoSep() << nb_bytes_src;
    dest << "out=" << OutputFormatFile::NoSep() << nb_bytes_dst;
    dest << "proto=" << OutputFormatFile::NoSep() << prot_term[1];

    dest << "src=" << OutputFormatFile::NoSep() << inet_ntop(AF_INET, &flow_tuple.client_ip.ipv4, buf, INET_ADDRSTRLEN);
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
        mac2[0], mac2[1], mac2[2], mac2[3], mac2[4], mac2[5]);

    dest << "smac=" << OutputFormatFile::NoSep() << mac_str;
    dest << "spt=" << OutputFormatFile::NoSep() << flow_tuple.client_port;
    dest << "start=" << OutputFormatFile::NoSep() << first_packet_timestamp;
}

}
