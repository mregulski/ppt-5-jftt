nl 		(\r?\n)
slash 	[/]
bslash 	[\\]
star 	[*]

%x string
%x comment
%x comment_multi
%x comment_doc
%x comment_doc_multi

%%
\"						BEGIN(string); ECHO;
<string>.				ECHO;
<string>{bslash}\"		ECHO;
<string>[^{bslash}]\"	ECHO; BEGIN(INITIAL);

{slash}{3}						BEGIN(comment_doc); ECHO;
<comment_doc>.*{bslash}{nl}		ECHO;
<comment_doc>[^{bslash]{nl}		ECHO; BEGIN(INITIAL);
<comment_doc>.					ECHO;

{slash}{star}{2}					BEGIN(comment_doc_multi); ECHO;
<comment_doc_multi>(.|{nl})			ECHO;
<comment_doc_multi>{star}{slash}	ECHO; BEGIN(INITIAL);

{slash}{slash}				BEGIN(comment);
<comment>.*{bslash}{nl}		;
<comment>[^{bslash}]{nl}	BEGIN(INITIAL); printf("\n");
<comment>.					; 

{slash}{star}					BEGIN(comment_multi);
<comment_multi>(.|{nl})			;
<comment_multi>{star}{slash}	BEGIN(INITIAL);

<<EOF>>		printf("\n"); return 0;
