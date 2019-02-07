#curl --header "Content-Type: application/json" --request POST --data {user: not_a_nasty_hacker, query: SELECT * FROM aids_patients WHERE name='someone_important'} http://127.0.0.1:5000
for(( i=0; i<=1000; i++ ))
do
    curl -X POST -H "Content-Type: application/json" -d '{"user":"Notorious$i","query":"SELECT * FROM Table"}' localhost:5000/write
done
