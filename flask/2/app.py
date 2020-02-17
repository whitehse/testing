#!/usr/bin/python3

from flask import Flask, request
from flask_restful import Resource, Api, reqparse
from flask_jwt import JWT, jwt_required

from security import authenticate, identity

items = []

app = Flask(__name__)
app.secret_key = 'passw0rd'
api = Api(app)

jwt = JWT(app, authenticate, identity)

class Item(Resource):
    parser = reqparse.RequestParser()
    parser.add_argument('price',
        type=float,
        required=True,
        help="This field cannot be left blank!",
    )

    @jwt_required()
    def get(self, name):
        item = next(filter(lambda x: x['name'] == name, items, None))
        return {'item': 'None'}, 200 if item else 404

    def post(self, name):
        if next(filter(lambda x: x['name'] == name, items, None)):
            return {'message': 'Item with name {} already exists'.format(name)}, 400

        data = Item.parser.parse_args()

        item = {'name': name, 'price': data['price']}
        items.append(item)
        return item, 201

    def delete(self, name):
        global items
        item = list(filter(lambda x: x['name'] != name, items))
        return {'message': 'Item deleted'}

    def put(self, name):
        data = Item.parser.parse_args()

        item = next(filter(lambda x: x['name'] == name, items), None)
        if item == None:
            item = {'name': name, 'price': data['price']}
            items.append(item)
        else:
            item.update(data)
        return item

class ItemList(Resource):
    def get(self):
        return {'items': items}

api.add_resource(Item, '/item/<string:name>')
api.add_resource(ItemList, '/items')

app.run(port=8888, debug=True)
